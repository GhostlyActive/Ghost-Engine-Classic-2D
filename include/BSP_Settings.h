#pragma once

#include <Arduino.h>      // PI
#include <stdint.h>


/* ============================================================================
 *  BSP_Settings.h
 *
 *  Single tuning surface for the BSP engine + game. Everything a player or
 *  developer might want to tweak lives here, as a typed `constexpr` value.
 *  The engine and game source files only read from these names; nothing
 *  else holds copies of them.
 *
 *  Sections:
 *    1) Display
 *    2) Map / BSP
 *    3) Renderer
 *    4) Gameplay (movement, rotation, spawn)
 *    5) Run-time buffers (extern declarations)
 *    6) BSP_Player struct
 * ============================================================================ */


/* ----- 1) Display -------------------------------------------------------- */

constexpr int screenWidth  = 128;
constexpr int screenHeight = 128;


/* ----- 2) Map / BSP ------------------------------------------------------ */

// Side length of the (square) world in arbitrary units.
//   400 -> sectors at iteration=6 are ~50 units wide and the world feels
//          cramped while walking past walls.
//   600 -> sectors ~75 units, walls breathe.
constexpr int map_Size = 600;

// Number of recursive BSP splits. The number of leaf sectors is 2^iteration.
//   iteration = 6  ->  64 sectors  ->  ~3 KB tree on AVR (fits in 8 KB SRAM).
//   iteration = 7  ->  128 sectors -> tight but doable.
//   iteration = 8  ->  256 sectors -> only with the slim 13-byte Container.
constexpr int iteration = 6;


/* ----- 3) Renderer ------------------------------------------------------- */

// Wall projection scale: lineHeight = WALL_PROJ_SCALE / depth.
// Rule of thumb:  WALL_PROJ_SCALE ~ (screenHeight / 2) * sectorSize.
// Lower values -> walls feel further away (less claustrophobic).
constexpr float WALL_PROJ_SCALE = 2400.0f;

// Near plane in world units. Wall edges with depth below this are clipped
// (or skipped if both endpoints are behind it). Stops "infinite line height"
// at depth == 0 and removes the ghost-stripe artifact for walls behind the
// camera.
constexpr float NEAR_PLANE_DEPTH = 1.0f;


/* ---- Floor / sky tinting ---- */
//
// 0 = solid black above and below walls (the original look).
// 1 = paint a sky tint on the upper half and a floor tint on the lower half.
//     Gives the world a horizon and makes rotation read more naturally,
//     because the eye has a stable up/down reference.
//
// Kept as a `#define` because it switches `#if` branches in the renderer.
#define ENABLE_FLOOR_AND_SKY 1

// RGB565 colors. Tweak to taste. These defaults mimic a dusk Doom palette.
constexpr uint16_t SKY_COLOR   = 0x29C7;   // soft slate blue-gray
constexpr uint16_t FLOOR_COLOR = 0x4208;   // muted dark brown


/* ---- HUD overlays ---- */
//
// Cosmetic overlays painted on top of the world each frame. Both are
// individually toggle-able. Disabling sets the compile-time branch to a
// no-op, so the engine pays zero runtime cost.

// FPS counter in the top-left corner (small white digits over black).
#define ENABLE_FPS_COUNTER  1

// Top-down map in the top-right corner showing the BSP sectors as colored
// rects, the player as a white dot, and the view direction as a yellow line.
#define ENABLE_MINIMAP      1

// Side length of the minimap in pixels.
constexpr int MINIMAP_SIZE = 32;


/* ----- 4) Gameplay ------------------------------------------------------- */

// Probability (0..100) that a leaf sector is randomly turned into a wall.
// 22 keeps the world open; raise toward 35-40 for a denser maze.
constexpr int WALL_DENSITY_PERCENT = 22;

// Player collision radius in world units. The collision check tests the
// player center plus four points at +/- this radius along X and Y, so the
// player cannot touch a wall face. Combined with the perspective projection
// at NEAR_PLANE_DEPTH, this prevents the "stuck inside the wall" feel.
constexpr int PLAYER_RADIUS = 16;

// Continuous turn / move rate while a hardware button is HELD (per frame).
// At ~30 FPS, HOLD_TURN_RATE = 0.06 rad gives ~100 deg/s, which feels
// natural without being twitchy.
constexpr float HOLD_TURN_RATE = 0.06f;
constexpr float HOLD_MOVE_RATE = 3.0f;

// Discrete step taken on a single Serial keystroke (w/a/s/d). Larger than
// the hold rate because each keystroke is a separate event - holding a key
// in the Serial Monitor only repeats at the OS auto-repeat rate.
constexpr float SERIAL_TURN_STEP = 0.18f;
constexpr float SERIAL_MOVE_STEP = 9.0f;

// Spawn point: world coordinates the player tries to spawn at. The actual
// spawn sector is the leaf containing this point; that sector is forced
// to be empty so every random wall is in front of the player.
constexpr int SPAWN_SEED_X = map_Size / 12;
constexpr int SPAWN_SEED_Y = map_Size / 12;

// Initial facing angle (radians). 7*PI/4 = -PI/4 in world convention points
// to +X, +Y, i.e. diagonally toward the opposite map corner.
constexpr float SPAWN_ANGLE = 7.0f * PI / 4.0f;


/* ----- 5) Run-time buffers ----------------------------------------------- */

// Per-column occlusion / delta-redraw buffers. Defined in BSP_Settings.cpp.
extern bool      xBuffer[screenWidth];     // was column drawn this frame?
extern int       yBuffer[screenWidth];     // last drawn line height per column
extern uint16_t  colorBuffer[screenWidth]; // last drawn color per column


/* ----- 6) BSP_Player ----------------------------------------------------- */

// Player position in world units. Movement and rotation rates are global
// constants now, not per-player fields, so this struct stays trivially small.
struct BSP_Player
{
    int player_px = SPAWN_SEED_X;
    int player_py = SPAWN_SEED_Y;
};
