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
// V.Container_map (the BSP tree) and V.test (the flat leaf array) hold
// separate copies of each leaf. Wall flags are kept in V.test, so any
// collision or wall query must use the tree to locate the sector and then
// jump into V.test via the 'order' index.
static bool sector_is_wall_at(int x, int y, View& V)
{
    Tree* sect = getPosition(x, y, V.Container_map);
    if (sect == NULL) return true;          // out of bounds -> treat as wall

    int idx = sect->leaf.order - 1;
    if (idx < 0 || idx >= V.rooms) return true;

    return V.test[idx].leaf.is_wall;
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


/* ----- "Out of RAM" screen ----------------------------------------------- */

static void out_of_ram_screen(Adafruit_SSD1351 tft)
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

void Game_start(Adafruit_SSD1351 tft)
{
    View V = Save_BSP_Engine();

    if (V.test == NULL || V.Container_map == NULL)
    {
        out_of_ram_screen(tft);
        buildMenu menu;
        menu.ShowMenu(tft);
        return;
    }

    BSP_Player P;

    // Pick the spawn sector (the one containing the corner seed point)
    // BEFORE scattering walls, so we can guarantee it stays empty.
    int spawnIdx = 0;
    {
        Tree* sn = getPosition(SPAWN_SEED_X, SPAWN_SEED_Y, V.Container_map);
        if (sn != NULL)
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
            V.test[i].leaf.is_wall = true;
            V.test[i].leaf.color   = WALL_COLORS[random(0, NUM_WALL_COLORS)];
        }
    }

    // Force the spawn sector empty (defensive: random() above can't have
    // touched it, but if any code path changes that this still wins).
    V.test[spawnIdx].leaf.is_wall = false;
    V.test[spawnIdx].leaf.color   = BLACK;

    P.player_px   = cx(V.test[spawnIdx].leaf);
    P.player_py   = cy(V.test[spawnIdx].leaf);
    V.playerAngle = SPAWN_ANGLE;

    Serial.print(F("[Scene] spawn idx=")); Serial.print(spawnIdx);
    Serial.print(F(" px="));               Serial.print(P.player_px);
    Serial.print(F(" py="));               Serial.println(P.player_py);

    Game_loop(tft, P, V);
}


/* ----- Render + input loop ---------------------------------------------- */

// Compute the player's forward direction in world coordinates from the
// current view angle. Convention: world angle = atan2(-dy, dx), so the Y
// axis is flipped vs. textbook math. See BSP_Engine.cpp DrawSegment for
// the matching projection.
static inline void forward_vector(float angle, float& fx, float& fy)
{
    fx =  cosf(angle);
    fy = -sinf(angle);
}


void Game_loop(Adafruit_SSD1351 tft, BSP_Player P, View V)
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
        RenderBSP(V.Container_map, tft, P, V);

        // Repaint the sky/floor over any column that lost its wall this
        // frame. Without this, last frame's wall would linger.
        for (int x = 0; x < screenWidth; x++)
        {
            if (xBuffer[x] == false)
            {
                if (yBuffer[x] != 0)
                {
                    int top    = (screenHeight - 1) / 2 - yBuffer[x] / 2;
                    int height = yBuffer[x] + 1;
                    paint_background_strip(tft, x, top, height);
                }
                yBuffer[x]     = 0;
                colorBuffer[x] = BLACK;
            }
        }
        memset(xBuffer, 0, screenWidth * sizeof(bool));

        // ----- Input -----
        btn_Bounce_F.update();
        btn_Bounce_B.update();
        btn_Bounce_L.update();
        btn_Bounce_R.update();

        // Forward direction in world frame.
        float fx, fy;
        forward_vector(V.playerAngle, fx, fy);

        // --- Forward / backward ---
        // Hardware buttons -> continuous (per-frame) walk while held.
        // Serial keystrokes -> single discrete step per key event.

        if (btn_Bounce_F.read() == LOW)
            try_move(P, V,
                     P.player_px + (int)(fx * HOLD_MOVE_RATE),
                     P.player_py + (int)(fy * HOLD_MOVE_RATE));

        if (Serial_F())
            try_move(P, V,
                     P.player_px + (int)(fx * SERIAL_MOVE_STEP),
                     P.player_py + (int)(fy * SERIAL_MOVE_STEP));

        if (btn_Bounce_B.read() == LOW)
            try_move(P, V,
                     P.player_px - (int)(fx * HOLD_MOVE_RATE),
                     P.player_py - (int)(fy * HOLD_MOVE_RATE));

        if (Serial_B())
            try_move(P, V,
                     P.player_px - (int)(fx * SERIAL_MOVE_STEP),
                     P.player_py - (int)(fy * SERIAL_MOVE_STEP));

        // --- Rotation ---
        // Right turn = decrease angle (world rotates left on screen, classic
        // FPS feel). Left turn = increase angle.

        if (btn_Bounce_R.read() == LOW) V.playerAngle -= HOLD_TURN_RATE;
        if (Serial_R())                 V.playerAngle -= SERIAL_TURN_STEP;

        if (btn_Bounce_L.read() == LOW) V.playerAngle += HOLD_TURN_RATE;
        if (Serial_L())                 V.playerAngle += SERIAL_TURN_STEP;
    }
}
