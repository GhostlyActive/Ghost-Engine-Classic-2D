#include "BSP_Engine.h"
#include <Arduino.h>


/* ============================================================================
 *  BSP_Engine.cpp
 *
 *  Sections:
 *      1) AVR free-RAM helper
 *      2) Map generation: BSP recursive split + leaf numbering
 *      3) Geometry helpers: VectorDegree
 *      4) Spatial query: getPosition
 *      5) Build entry point: Save_BSP_Engine
 *      6) Renderer: FillSegment, DrawSegment, DrawWall, RenderBSP
 *      7) View helpers: FieldOfView2Angle
 *
 *  See BSP_Engine.h for the API and BSP_Settings.h for tunables.
 * ============================================================================ */


/* ============================================================================
 *  1) Free-RAM helper for AVR
 * ============================================================================ */

extern int  __heap_start;
extern int* __brkval;

static int free_ram()
{
    int local;
    return (int)&local - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}


/* ============================================================================
 *  2) Map generation
 * ============================================================================ */

Tree* newLeaf(Container leaf)
{
    Tree* node = (Tree*)malloc(sizeof(Tree));
    if (node == NULL) return NULL;
    node->leaf   = leaf;
    node->lchild = NULL;
    node->rchild = NULL;
    return node;
}


// Split a container into two halves (vertical or horizontal). Iterative,
// not recursive: thin containers used to make the recursive version blow
// the stack. Up to 16 randomised attempts, then a forced split.
Array2 geo_split(Container* container)
{
    Array2 two;

    for (int tries = 0; tries < 16; tries++)
    {
        int direction = random(0, 2);

        if (direction == 0)
        {
            // Vertical split: cut along the X axis.
            two.array[0].x = container->x;
            two.array[0].y = container->y;
            two.array[0].w = container->w / 2;
            two.array[0].h = container->h;

            two.array[1].x = container->x + two.array[0].w;
            two.array[1].y = container->y;
            two.array[1].w = container->w - two.array[0].w;
            two.array[1].h = container->h;

            float r1 = (float)two.array[0].w / (float)two.array[0].h;
            float r2 = (float)two.array[1].w / (float)two.array[1].h;
            if (r1 >= 0.5f && r2 >= 0.5f) return two;
        }
        else
        {
            // Horizontal split: cut along the Y axis.
            two.array[0].x = container->x;
            two.array[0].y = container->y;
            two.array[0].w = container->w;
            two.array[0].h = container->h / 2;

            two.array[1].x = container->x;
            two.array[1].y = container->y + two.array[0].h;
            two.array[1].w = container->w;
            two.array[1].h = container->h - two.array[0].h;

            float r1 = (float)two.array[0].h / (float)two.array[0].w;
            float r2 = (float)two.array[1].h / (float)two.array[1].w;
            if (r1 >= 1.0f && r2 >= 1.0f) return two;
        }
    }

    // Fallback: container is too skewed for the ratio constraint. Force
    // a vertical split. Should not happen with map_Size = 600 and iteration
    // <= 8, but better to return something valid than to spin forever.
    two.array[0].x = container->x;
    two.array[0].y = container->y;
    two.array[0].w = container->w / 2;
    two.array[0].h = container->h;
    two.array[1].x = container->x + two.array[0].w;
    two.array[1].y = container->y;
    two.array[1].w = container->w - two.array[0].w;
    two.array[1].h = container->h;
    return two;
}


// Recursively split a container `iter` times, building the BSP tree.
Tree* split_container(Container container, int iter)
{
    Tree* root = newLeaf(container);
    if (root == NULL) return NULL;

    if (iter != 0)
    {
        Array2 sr = geo_split(&container);
        root->lchild = split_container(sr.array[0], iter - 1);
        root->rchild = split_container(sr.array[1], iter - 1);
    }

    return root;
}


// Walk the tree in-order and copy every leaf into `order[]`, assigning each
// a running number 1..rooms. The number is also written back to the tree
// so getPosition() can produce the same index.
Tree* printLeafNodes(Tree* root, Tree* order, int* step)
{
    if (!root) return order;

    // Leaf?
    if (!root->lchild && !root->rchild)
    {
        root->leaf.order      = *step;
        order[*step - 1].leaf = root->leaf;
        (*step)++;
        return order;
    }

    if (root->lchild) printLeafNodes(root->lchild, order, step);
    if (root->rchild) printLeafNodes(root->rchild, order, step);

    return order;
}


/* ============================================================================
 *  3) Geometry: world-angle from one point to another
 * ============================================================================ */

// Returns the angle of the vector (x_wall, y_wall) -> (x2_wall, y2_wall) in
// degrees, normalised to [0, 360). The Y axis is flipped (atan2 of -dy)
// because the screen has +Y pointing down. With this convention:
//      0 deg   = +X  (east)
//      90 deg  = -Y  (screen up)
//      180 deg = -X  (west)
//      270 deg = +Y  (screen down)
float VectorDegree(int& x_wall, int& y_wall, int& x2_wall, int& y2_wall)
{
    float angle = atan2(-y2_wall + y_wall, x2_wall - x_wall) * 180.0f / PI;
    angle = angle + ceil(-angle / 360.0f) * 360.0f;     // wrap into [0, 360)
    return angle;
}


/* ============================================================================
 *  4) Spatial query
 * ============================================================================ */

// Find the leaf node whose AABB contains (value_x, value_y). Recursive
// descent of the BSP tree.
Tree* getPosition(int value_x, int value_y, Tree* node)
{
    if (node->lchild == NULL && node->rchild == NULL)
        return node;

    if (node->lchild != NULL &&
        node->lchild->leaf.x <= value_x &&
        (node->lchild->leaf.x + node->lchild->leaf.w) >= value_x &&
        node->lchild->leaf.y <= value_y &&
        (node->lchild->leaf.y + node->lchild->leaf.h) >= value_y)
    {
        return getPosition(value_x, value_y, node->lchild);
    }

    if (node->rchild != NULL &&
        node->rchild->leaf.x <= value_x &&
        (node->rchild->leaf.x + node->rchild->leaf.w) >= value_x &&
        node->rchild->leaf.y <= value_y &&
        (node->rchild->leaf.y + node->rchild->leaf.h) >= value_y)
    {
        return getPosition(value_x, value_y, node->rchild);
    }

    return NULL;
}


/* ============================================================================
 *  5) Build entry point
 * ============================================================================ */

View Save_BSP_Engine()
{
    View v;

    Serial.print(F("[BSP] iteration="));    Serial.print(iteration);
    Serial.print(F(" rooms="));             Serial.print(v.rooms);
    Serial.print(F(" sizeof(Tree)="));      Serial.print((int)sizeof(Tree));
    Serial.print(F(" freeRAM_pre="));       Serial.println(free_ram());

    Tree* leaves = new Tree[v.rooms];

    if (leaves == NULL)
    {
        Serial.println(F("[BSP] ERROR: 'new Tree[rooms]' returned NULL - out of RAM."));
        v.test          = NULL;
        v.Container_map = NULL;
        return v;
    }

    Container root_container;
    root_container.x = 0;
    root_container.y = 0;
    root_container.w = map_Size;
    root_container.h = map_Size;

    v.Container_map = split_container(root_container, iteration);

    if (v.Container_map == NULL)
    {
        Serial.println(F("[BSP] ERROR: split_container returned NULL - out of RAM."));
        delete[] leaves;
        v.test = NULL;
        return v;
    }

    int step = 1;
    v.test = printLeafNodes(v.Container_map, leaves, &step);

    Serial.print(F("[BSP] freeRAM_post=")); Serial.println(free_ram());
    return v;
}


/* ============================================================================
 *  6) Renderer
 * ============================================================================ */


/* ----- 6a) FillSegment --------------------------------------------------
 *
 * Writes one wall column strip to the display.
 *
 * Inputs:
 *      dist_1, dist_2     perpendicular depth at each screen X endpoint
 *      x_pos_1, x_pos_2   screen X of each endpoint
 *
 * For every column from min(x_pos) to max(x_pos):
 *      lineHeight = WALL_PROJ_SCALE / depth          (perspective-correct)
 *      depth interpolated linearly in 1/depth        (NOT in depth itself,
 *                                                      that would warp)
 * ---------------------------------------------------------------------- */
void FillSegment(float dist_1, float dist_2,
                 int x_pos_1, int x_pos_2,
                 uint16_t color,
                 Adafruit_SSD1351 tft, BSP_Player P, View V)
{
    // Endpoints can arrive in either screen-X order. Without swapping the
    // for-loop below would run zero times -> visible gap on the wall.
    if (x_pos_1 > x_pos_2)
    {
        int   tx = x_pos_1; x_pos_1 = x_pos_2; x_pos_2 = tx;
        float td = dist_1;  dist_1  = dist_2;  dist_2  = td;
    }

    int pos_diff = x_pos_2 - x_pos_1;
    if (pos_diff == 0) pos_diff = 1;        // 1-column wall, avoid div by zero

    // Clamp depth to >= 1 so 1/depth never blows up. With near-plane clipping
    // in DrawSegment, depths are already >= NEAR_PLANE_DEPTH; this is the
    // last safety net.
    const float minD = 1.0f;
    float inv_d_1 = 1.0f / (dist_1 < minD ? minD : dist_1);
    float inv_d_2 = 1.0f / (dist_2 < minD ? minD : dist_2);

    float inv_step = (inv_d_2 - inv_d_1) / (float)pos_diff;
    float inv_d    = inv_d_1;

    const int screenH = (int)(screenHeight - screenStatistics);

    for (int x = x_pos_1; x <= x_pos_2; x++)
    {
        int lineHeight = (int)(WALL_PROJ_SCALE * inv_d);
        if (lineHeight > screenH) lineHeight = screenH;
        if (lineHeight < 0)       lineHeight = 0;

        // xBuffer occlusion: if a closer wall already filled this column,
        // skip. This is the per-column z-buffer used by Doom.
        if (xBuffer[x] == false)
        {
            // -- Delta redraw -- only push to SPI when something actually
            // changed since last frame. Big speedup at low rotation rates.
            //
            // Walls are centered vertically: top = (screenH - lineHeight)/2,
            // bottom = top + lineHeight. The strip above the wall is the
            // sky region; the strip below is the floor region.
            if (yBuffer[x] != lineHeight)
            {
                if (colorBuffer[x] == color)
                {
                    // Same color, only height changed: paint the difference.
                    int delta   = yBuffer[x] - lineHeight;
                    int top_old = (screenH - yBuffer[x])  / 2;
                    int top_new = (screenH - lineHeight)  / 2;

                    if (delta > 0)
                    {
                        // Wall got smaller -> reveal sky/floor strips.
                        // Above-wall strip is fully in the sky region;
                        // below-wall strip is fully in the floor region
                        // (walls are centered around screenH/2).
#if ENABLE_FLOOR_AND_SKY
                        tft.drawFastVLine(x, top_old,           top_new - top_old, SKY_COLOR);
                        tft.drawFastVLine(x, screenH - top_new, top_new - top_old, FLOOR_COLOR);
#else
                        tft.drawFastVLine(x, top_old,           top_new - top_old, BLACK);
                        tft.drawFastVLine(x, screenH - top_new, top_new - top_old, BLACK);
#endif
                    }
                    else
                    {
                        // Wall got bigger -> paint wall over the revealed strips.
                        tft.drawFastVLine(x, top_new, top_old - top_new, color);
                        int begin_y = screenH - top_new - (top_old - top_new) - 1;
                        tft.drawFastVLine(x, begin_y, top_old - top_new, color);
                    }
                }
                else
                {
                    // Different color and different height -> repaint the
                    // entire column: sky on top, wall in the middle, floor
                    // on the bottom.
                    int top    = (screenH - lineHeight) / 2;
                    int bottom = top + lineHeight;

                    paint_background_strip(tft, x, 0,      top);
                    tft.drawFastVLine(x, top, lineHeight, color);
                    paint_background_strip(tft, x, bottom, screenH - bottom);
                }
            }
            else if (colorBuffer[x] != color)
            {
                // Same height, just a color change. Wall area only.
                int top = (screenH - lineHeight) / 2;
                tft.drawFastVLine(x, top, lineHeight, color);
            }

            yBuffer[x]     = lineHeight;
            colorBuffer[x] = color;
            xBuffer[x]     = true;
        }

        inv_d += inv_step;
    }
}


/* ----- 6b) DrawSegment --------------------------------------------------
 *
 * Projects a wall edge (two world points) into screen space and hands the
 * result to FillSegment. The pipeline:
 *
 *   1) Transform endpoints into camera space.
 *           forward = ( cos(angle), -sin(angle) )       (Y axis is screen-flipped)
 *           right   = ( sin(angle),  cos(angle) )
 *           depth   = v . forward
 *           lateral = v . right
 *
 *   2) Clip the segment against three half-spaces (Sutherland-Hodgman).
 *           - near plane    :  depth >= NEAR_PLANE_DEPTH
 *           - right FOV ray :  depth * tan(FOV/2) - lat >= 0
 *           - left  FOV ray :  depth * tan(FOV/2) + lat >= 0
 *      If the segment is fully outside any plane, skip the wall.
 *      Otherwise endpoints outside the plane are linearly interpolated to
 *      the plane.
 *
 *   3) Project the (now in-FOV, in-front-of-camera) endpoints to screen X:
 *           screen_x = w/2 + (lateral / depth) / tan(FOV/2) * (w/2)
 *
 *   4) Hand off to FillSegment.
 *
 * Why a full FOV clip and not just a near-plane clip:
 *   With only a near-plane clip, walls that extend past the side of the
 *   FOV got pulled forward to depth = NEAR_PLANE = 1, which then projected
 *   to the screen edge with line height SCALE / 1 = full screen. Walls
 *   along the side appeared "in your face" and their height collapsed to
 *   the actual wall depth only well inside the screen, giving the
 *   "wall stretched into the middle" look.
 *
 *   FOV-cone clipping puts the side endpoint exactly where the wall
 *   crosses the FOV ray, so the line height at the screen edge is the
 *   correct depth at that ray, not 1.
 * ---------------------------------------------------------------------- */

// Clip a wall segment in (depth, lat) camera space against one half-space.
//   inside region  =  { (d, l) : A*d + B*l + C >= 0 }
// Returns false if the segment is entirely outside (caller should skip the
// wall). Endpoints outside the half-space are linearly moved onto the plane.
static bool clip_plane(float& d1, float& l1,
                       float& d2, float& l2,
                       float A, float B, float C)
{
    float s1 = A * d1 + B * l1 + C;
    float s2 = A * d2 + B * l2 + C;
    if (s1 < 0 && s2 < 0) return false;
    if (s1 < 0)
    {
        float t = s1 / (s1 - s2);
        d1 += t * (d2 - d1);
        l1 += t * (l2 - l1);
    }
    else if (s2 < 0)
    {
        float t = s2 / (s2 - s1);
        d2 += t * (d1 - d2);
        l2 += t * (l1 - l2);
    }
    return true;
}


void DrawSegment(xy edge1, xy edge2,
                 uint16_t color,
                 Adafruit_SSD1351 tft, BSP_Player P, View V)
{
    // (1) world -> camera space.
    float vx1 = (float)edge1.x - (float)P.player_px;
    float vy1 = (float)edge1.y - (float)P.player_py;
    float vx2 = (float)edge2.x - (float)P.player_px;
    float vy2 = (float)edge2.y - (float)P.player_py;

    float fx =  cosf(V.playerAngle);
    float fy = -sinf(V.playerAngle);
    float rx =  sinf(V.playerAngle);
    float ry =  cosf(V.playerAngle);

    float depth1 = vx1 * fx + vy1 * fy;
    float lat1   = vx1 * rx + vy1 * ry;
    float depth2 = vx2 * fx + vy2 * fy;
    float lat2   = vx2 * rx + vy2 * ry;

    // (2) clip against the three planes of the camera frustum.
    const float tan_half = tanf(V.FOV * 0.5f);

    // near plane:  depth - NEAR_PLANE_DEPTH >= 0
    if (!clip_plane(depth1, lat1, depth2, lat2, 1.0f, 0.0f, -NEAR_PLANE_DEPTH))
        return;

    // right FOV: depth*tan_half - lat >= 0  (lat <= +tan_half * depth)
    if (!clip_plane(depth1, lat1, depth2, lat2, tan_half, -1.0f, 0.0f))
        return;

    // left  FOV: depth*tan_half + lat >= 0  (lat >= -tan_half * depth)
    if (!clip_plane(depth1, lat1, depth2, lat2, tan_half, 1.0f, 0.0f))
        return;

    // (3) project to screen X.
    const float halfW = (float)screenWidth * 0.5f;
    const float invTanHalfF = 1.0f / tan_half;

    int pos1 = (int)(halfW + (lat1 / depth1) * halfW * invTanHalfF);
    int pos2 = (int)(halfW + (lat2 / depth2) * halfW * invTanHalfF);

    // Defensive clamp - after clipping these should already be in range,
    // but float rounding can bite at the FOV edge.
    if (pos1 < 0)               pos1 = 0;
    if (pos1 > screenWidth - 1) pos1 = screenWidth - 1;
    if (pos2 < 0)               pos2 = 0;
    if (pos2 > screenWidth - 1) pos2 = screenWidth - 1;

    // (4) draw.
    FillSegment(depth1, depth2, pos1, pos2, color, tft, P, V);
}


/* ----- 6c) DrawWall -----------------------------------------------------
 *
 * For a single BSP leaf containing a wall, work out which 1-2 sides face
 * the player and draw them. There are 8 octant cases; the layout below
 * mirrors them in code order.
 * ---------------------------------------------------------------------- */
void DrawWall(Tree* wall, Adafruit_SSD1351 tft, BSP_Player P, View V)
{
    if (!wall->leaf.is_wall) return;

    const Container& W = wall->leaf;

    if (P.player_px <= W.x)
    {
        // Player is to the LEFT of the wall.

        if (P.player_py > (W.y + W.h))
        {
            // Player is below-left  -> see LEFT face + BOTTOM face.
            DrawSegment(edge_lu(W), edge_ld(W), W.color, tft, P, V);
            DrawSegment(edge_ld(W), edge_rd(W), W.color, tft, P, V);
        }
        else if (P.player_py < W.y)
        {
            // Player is above-left  -> see LEFT face + TOP face.
            DrawSegment(edge_lu(W), edge_ld(W), W.color, tft, P, V);
            DrawSegment(edge_ru(W), edge_lu(W), W.color, tft, P, V);
        }
        else
        {
            // Player is due-left    -> see LEFT face only.
            DrawSegment(edge_lu(W), edge_ld(W), W.color, tft, P, V);
        }
    }
    else if (P.player_px > (W.x + W.w))
    {
        // Player is to the RIGHT of the wall.

        if (P.player_py > (W.y + W.h))
        {
            // Player is below-right -> see RIGHT face + BOTTOM face.
            DrawSegment(edge_rd(W), edge_ru(W), W.color, tft, P, V);
            DrawSegment(edge_ld(W), edge_rd(W), W.color, tft, P, V);
        }
        else if (P.player_py < W.y)
        {
            // Player is above-right -> see RIGHT face + TOP face.
            DrawSegment(edge_rd(W), edge_ru(W), W.color, tft, P, V);
            DrawSegment(edge_ru(W), edge_lu(W), W.color, tft, P, V);
        }
        else
        {
            // Player is due-right   -> see RIGHT face only.
            DrawSegment(edge_rd(W), edge_ru(W), W.color, tft, P, V);
        }
    }
    else
    {
        // Player's X is between the wall's X edges.

        if (P.player_py <= W.y)
        {
            // Player is straight ABOVE -> see TOP face.
            DrawSegment(edge_ru(W), edge_lu(W), W.color, tft, P, V);
        }
        else if (P.player_py >= (W.y + W.h))
        {
            // Player is straight BELOW -> see BOTTOM face.
            DrawSegment(edge_ld(W), edge_rd(W), W.color, tft, P, V);
        }
        // else: player is INSIDE the wall sector. Should not happen with
        // PLAYER_RADIUS collision; if it does, draw nothing rather than
        // guessing which face is "outside".
    }
}


/* ----- 6d) RenderBSP ----------------------------------------------------
 *
 * True Doom-style front-to-back BSP traversal. At every inner node:
 *      same X -> horizontal split (Y-axis)
 *      same Y -> vertical split   (X-axis)
 * Render the side the player is on first, then the far side. Combined
 * with the per-column xBuffer, this gives correct visibility without
 * any explicit depth sorting.
 *
 * IMPORTANT: at a leaf we look up the wall data in V.test (the flat
 * leaves array), not in Container_map - that's where the random walls
 * were written by Game_start.
 * ---------------------------------------------------------------------- */
void RenderBSP(Tree* node, Adafruit_SSD1351 tft, BSP_Player P, View V)
{
    if (node == NULL) return;

    if (node->lchild == NULL && node->rchild == NULL)
    {
        int idx = node->leaf.order - 1;
        if (idx >= 0 && idx < V.rooms) DrawWall(&V.test[idx], tft, P, V);
        return;
    }

    Tree* L = node->lchild;
    Tree* R = node->rchild;

    if (L == NULL || R == NULL)
    {
        if (L != NULL) RenderBSP(L, tft, P, V);
        if (R != NULL) RenderBSP(R, tft, P, V);
        return;
    }

    bool nearIsLeft;
    if (L->leaf.x != R->leaf.x)
        nearIsLeft = (P.player_px < R->leaf.x);   // vertical split at x = R.x
    else
        nearIsLeft = (P.player_py < R->leaf.y);   // horizontal split at y = R.y

    if (nearIsLeft)
    {
        RenderBSP(L, tft, P, V);   // near side first
        RenderBSP(R, tft, P, V);   // far side, occluded by xBuffer
    }
    else
    {
        RenderBSP(R, tft, P, V);
        RenderBSP(L, tft, P, V);
    }
}


/* ============================================================================
 *  7) View helpers
 * ============================================================================ */

void FieldOfView2Angle(View& V)
{
    V.playerAngle = fmodf(V.playerAngle, 2.0f * PI);
    if (V.playerAngle < 0) V.playerAngle += 2.0f * PI;
}


/* ============================================================================
 *  8) Background painting (sky / floor)
 * ============================================================================ */

void paint_background_strip(Adafruit_SSD1351 tft, int x, int y, int h)
{
    if (h <= 0) return;

#if ENABLE_FLOOR_AND_SKY
    const int mid    = screenHeight / 2;
    const int bottom = y + h;

    if (bottom <= mid)
    {
        tft.drawFastVLine(x, y, h, SKY_COLOR);
    }
    else if (y >= mid)
    {
        tft.drawFastVLine(x, y, h, FLOOR_COLOR);
    }
    else
    {
        // Strip straddles the horizon: paint two sub-strips.
        tft.drawFastVLine(x, y,    mid - y,      SKY_COLOR);
        tft.drawFastVLine(x, mid,  bottom - mid, FLOOR_COLOR);
    }
#else
    tft.drawFastVLine(x, y, h, BLACK);
#endif
}


void paint_full_background(Adafruit_SSD1351 tft)
{
#if ENABLE_FLOOR_AND_SKY
    const int mid = screenHeight / 2;
    tft.fillRect(0, 0,   screenWidth, mid,                 SKY_COLOR);
    tft.fillRect(0, mid, screenWidth, screenHeight - mid,  FLOOR_COLOR);
#else
    tft.fillScreen(BLACK);
#endif
}
