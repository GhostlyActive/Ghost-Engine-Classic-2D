#include "Game.h"
#include "Menu.h"
#include "Controls.h"


/* ============================================================================
 *  Game.cpp
 *
 *  Glue between the BSP renderer (BSP_Engine) and the input / world logic.
 *  All tunable numbers live in BSP_Settings.h.
 *
 *  Frame loop:
 *      Serial_poll()           // drain Serial -> one-shot flags
 *      FieldOfView2Angle(V)    // wrap player angle into [0, 2*PI)
 *      RenderBSP()             // draw the world for this frame
 *      column cleanup          // erase stale columns
 *      input -> movement       // hold-poll buttons + Serial keystrokes
 * ============================================================================ */


/* ----- Wall palette ------------------------------------------------------ */

static const uint16_t WALL_COLORS[] = { RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA };
static const int      NUM_WALL_COLORS = sizeof(WALL_COLORS) / sizeof(WALL_COLORS[0]);


/* ----- Wall lookup ------------------------------------------------------- */
//
// V.bspRoot (the BSP tree) and V.leaves (the flat leaf array) hold
// separate copies of each leaf. Wall flags are kept in V.leaves, so any
// collision or wall query must use the tree to locate the sector and then
// jump into V.leaves via the 'order' index.
static bool sector_is_wall_at(int x, int y, View& V)
{
    Tree* sect = getPosition(x, y, V.bspRoot);
    if (sect == nullptr) return true;          // out of bounds -> treat as wall

    int idx = sect->leaf.order - 1;
    if (idx < 0 || idx >= V.rooms) return true;

    return V.leaves[idx].leaf.is_wall;
}


// Player has a non-zero collision radius. The proposed center is rejected
// if either the center itself or any of the four cardinal points at
// +/- PLAYER_RADIUS lies inside a wall sector. This keeps the camera at
// least PLAYER_RADIUS units away from any wall face -> no "glued to the
// wall" perspective and no clipping through corners.
static bool player_blocked(int x, int y, View& V)
{
    if (sector_is_wall_at(x,                    y,                    V)) return true;
    if (sector_is_wall_at(x + PLAYER_RADIUS,    y,                    V)) return true;
    if (sector_is_wall_at(x - PLAYER_RADIUS,    y,                    V)) return true;
    if (sector_is_wall_at(x,                    y + PLAYER_RADIUS,    V)) return true;
    if (sector_is_wall_at(x,                    y - PLAYER_RADIUS,    V)) return true;
    return false;
}


// Try to move the player to (new_x, new_y). If blocked, slide along the
// blocking axis (move just X or just Y). If everything is blocked, stay put.
// This is the classic AABB slide collision response and prevents the
// "stuck in a corner" feeling on diagonal motion.
static void try_move(BSP_Player& P, View& V, int new_x, int new_y)
{
    if (!player_blocked(new_x, new_y, V))
    {
        P.player_px = new_x;
        P.player_py = new_y;
    }
    else if (!player_blocked(new_x, P.player_py, V))
    {
        P.player_px = new_x;
    }
    else if (!player_blocked(P.player_px, new_y, V))
    {
        P.player_py = new_y;
    }
}


/* ----- HUD overlays ------------------------------------------------------ */
//
// Drawn AFTER the world each frame, so they always sit on top. Both are
// compile-time gated by the toggles in BSP_Settings.h - when disabled the
// preprocessor strips them out completely.

#if ENABLE_MINIMAP
// Top-down minimap in the upper-right corner. Each BSP leaf that is a wall
// is drawn as a small filled rect in its wall color. The player is a white
// dot with a short yellow line in the view direction.
static void draw_minimap(Adafruit_SSD1351& tft, const BSP_Player& P, const View& V)
{
    constexpr int size = MINIMAP_SIZE;
    constexpr int ox   = screenWidth - size;
    constexpr int oy   = 0;

    // Background + frame.
    tft.fillRect(ox, oy, size, size, BLACK);
    tft.drawRect(ox, oy, size, size, WHITE);

    // Walls (scaled to mini space).
    for (int i = 0; i < V.rooms; ++i)
    {
        const Container& s = V.leaves[i].leaf;
        if (!s.is_wall) continue;

        const int x = ox + static_cast<int>(static_cast<int32_t>(s.x) * size / map_Size);
        const int y = oy + static_cast<int>(static_cast<int32_t>(s.y) * size / map_Size);
        int w       = static_cast<int>(static_cast<int32_t>(s.w) * size / map_Size);
        int h       = static_cast<int>(static_cast<int32_t>(s.h) * size / map_Size);
        if (w < 1) w = 1;
        if (h < 1) h = 1;
        tft.fillRect(x, y, w, h, s.color);
    }

    // Player position + view direction.
    const int px = ox + static_cast<int>(static_cast<int32_t>(P.player_px) * size / map_Size);
    const int py = oy + static_cast<int>(static_cast<int32_t>(P.player_py) * size / map_Size);

    constexpr int line_len = 5;
    const int ex = px + static_cast<int>( cosf(V.playerAngle) * line_len);
    const int ey = py + static_cast<int>(-sinf(V.playerAngle) * line_len);

    tft.drawLine(px, py, ex, ey, YELLOW);
    tft.fillRect(px - 1, py - 1, 2, 2, WHITE);
}
#endif


#if ENABLE_FPS_COUNTER
// Updates an internal counter every frame; recomputes the displayed value
// roughly twice a second. The text strip is always redrawn (cheap, ~24*9
// pixels) so a wall sliding into the corner cannot half-eat the digits.
static void draw_fps(Adafruit_SSD1351& tft)
{
    constexpr int box_w = 24;
    constexpr int box_h = 9;
    constexpr unsigned long window_ms = 500;

    static unsigned long window_start = 0;
    static int           frame_count  = 0;
    static int           displayed    = 0;

    ++frame_count;
    const unsigned long now = millis();
    if (window_start == 0) window_start = now;            // first call
    const unsigned long elapsed = now - window_start;
    if (elapsed >= window_ms)
    {
        displayed    = static_cast<int>(
                          static_cast<uint32_t>(frame_count) * 1000UL / elapsed);
        window_start = now;
        frame_count  = 0;
    }

    tft.fillRect(0, 0, box_w, box_h, BLACK);
    tft.setCursor(1, 1);
    tft.setTextSize(1);
    tft.setTextColor(WHITE);
    tft.print(displayed);
}
#endif


/* ----- "Out of RAM" screen ----------------------------------------------- */

static void out_of_ram_screen(Adafruit_SSD1351& tft)
{
    tft.fillScreen(BLACK);
    tft.drawRect(0, 0, screenWidth, screenHeight, RED);
    tft.setTextColor(RED);
    tft.setTextSize(2);
    tft.setCursor(8, (screenHeight / 2) - 20);
    tft.println("OUT OF");
    tft.setCursor(40, (screenHeight / 2));
    tft.println("RAM");
    tft.setTextColor(YELLOW);
    tft.setTextSize(1);
    tft.setCursor(8, (screenHeight / 2) + 25);
    tft.print("iteration=");
    tft.println(iteration);
    delay(2500);
}


/* ----- Game start: build world + place player ---------------------------- */

void Game_start(Adafruit_SSD1351& tft)
{
    View V = Save_BSP_Engine();

    if (V.leaves == nullptr || V.bspRoot == nullptr)
    {
        out_of_ram_screen(tft);
        Menu menu;
        menu.show(tft);
        return;
    }

    BSP_Player P;

    // Pick the spawn sector (the one containing the corner seed point)
    // BEFORE scattering walls, so we can guarantee it stays empty.
    int spawnIdx = 0;
    {
        Tree* sn = getPosition(SPAWN_SEED_X, SPAWN_SEED_Y, V.bspRoot);
        if (sn != nullptr)
        {
            int idx = sn->leaf.order - 1;
            if (idx >= 0 && idx < V.rooms) spawnIdx = idx;
        }
    }

    // Scatter random walls, skipping the spawn sector.
    for (int i = 0; i < V.rooms; i++)
    {
        if (i == spawnIdx) continue;

        if (random(0, 100) < WALL_DENSITY_PERCENT)
        {
            V.leaves[i].leaf.is_wall = true;
            V.leaves[i].leaf.color   = WALL_COLORS[random(0, NUM_WALL_COLORS)];
        }
    }

    // Force the spawn sector empty (defensive: random() above can't have
    // touched it, but if any code path changes that this still wins).
    V.leaves[spawnIdx].leaf.is_wall = false;
    V.leaves[spawnIdx].leaf.color   = BLACK;

    P.player_px   = V.leaves[spawnIdx].leaf.cx();
    P.player_py   = V.leaves[spawnIdx].leaf.cy();
    V.playerAngle = SPAWN_ANGLE;

    Serial.print(F("[Scene] spawn idx=")); Serial.print(spawnIdx);
    Serial.print(F(" px="));               Serial.print(P.player_px);
    Serial.print(F(" py="));               Serial.println(P.player_py);

    Game_loop(tft, P, V);
}


/* ----- Render + input loop ---------------------------------------------- */

// Player's forward unit vector in world coordinates from the current view
// angle. World angle convention (atan2(-dy, dx)) means Y axis is flipped
// vs. textbook math, so the y component carries the minus sign. See
// BSP_Engine.cpp DrawSegment for the matching projection.
struct Vec2f { float x, y; };
static constexpr Vec2f forward_dir(float angle)
{
    return { cosf(angle), -sinf(angle) };
}


void Game_loop(Adafruit_SSD1351& tft, BSP_Player P, View V)
{
    Btn_setup();

    // Lay down the initial sky + floor (or solid black if disabled). All
    // subsequent frames are delta-redraws on top of this background.
    paint_full_background(tft);

    for (;;)
    {
        // ----- Per-frame state -----
        Serial_poll();
        FieldOfView2Angle(V);

        // ----- Render -----
        RenderBSP(V.bspRoot, tft, P, V);

        // Repaint the sky/floor over any column that lost its wall this
        // frame. Without this, last frame's wall would linger.
        for (int x = 0; x < screenWidth; ++x)
        {
            if (!xBuffer[x])
            {
                if (yBuffer[x] != 0)
                {
                    const int top    = (screenHeight - 1) / 2 - yBuffer[x] / 2;
                    const int height = yBuffer[x] + 1;
                    paint_background_strip(tft, x, top, height);
                }
                yBuffer[x]     = 0;
                colorBuffer[x] = BLACK;
            }
        }
        memset(xBuffer, 0, screenWidth * sizeof(bool));

        // ----- HUD overlays -----
#if ENABLE_MINIMAP
        draw_minimap(tft, P, V);
#endif
#if ENABLE_FPS_COUNTER
        draw_fps(tft);
#endif

        // ----- Input -----
        btn_Bounce_F.update();
        btn_Bounce_B.update();
        btn_Bounce_L.update();
        btn_Bounce_R.update();

        const Vec2f fwd = forward_dir(V.playerAngle);

        // Forward / backward.
        //   Hardware button held -> continuous (per-frame) walk.
        //   Serial keystroke     -> single discrete step.
        if (btn_Bounce_F.read() == LOW)
            try_move(P, V,
                     P.player_px + static_cast<int>(fwd.x * HOLD_MOVE_RATE),
                     P.player_py + static_cast<int>(fwd.y * HOLD_MOVE_RATE));
        if (Serial_F())
            try_move(P, V,
                     P.player_px + static_cast<int>(fwd.x * SERIAL_MOVE_STEP),
                     P.player_py + static_cast<int>(fwd.y * SERIAL_MOVE_STEP));

        if (btn_Bounce_B.read() == LOW)
            try_move(P, V,
                     P.player_px - static_cast<int>(fwd.x * HOLD_MOVE_RATE),
                     P.player_py - static_cast<int>(fwd.y * HOLD_MOVE_RATE));
        if (Serial_B())
            try_move(P, V,
                     P.player_px - static_cast<int>(fwd.x * SERIAL_MOVE_STEP),
                     P.player_py - static_cast<int>(fwd.y * SERIAL_MOVE_STEP));

        // Rotation. Right turn = decrease angle (world rotates left on
        // screen, classic FPS feel). Left turn = increase angle.
        if (btn_Bounce_R.read() == LOW) V.playerAngle -= HOLD_TURN_RATE;
        if (Serial_R())                 V.playerAngle -= SERIAL_TURN_STEP;

        if (btn_Bounce_L.read() == LOW) V.playerAngle += HOLD_TURN_RATE;
        if (Serial_L())                 V.playerAngle += SERIAL_TURN_STEP;
    }
}
