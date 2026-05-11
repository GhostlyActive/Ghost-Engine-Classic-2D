#ifndef BSP_SETTINGS_H
#define BSP_SETTINGS_H

#include <Arduino.h>      // for PI
#include <stdint.h>

/* ============================================================================
 *  BSP_Settings.h
 *
 *  Single tuning surface for the BSP engine + game. Everything that a player
 *  or developer might want to tweak lives here. The engine and game source
 *  files only read from these macros; nothing else holds copies of these
 *  values.
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

#define screenWidth        128
#define screenHeight       128


/* ----- 2) Map / BSP ------------------------------------------------------ */

// Side length of the (square) world in arbitrary units.
//   400 -> sectors at iteration=6 are ~50 units wide and the world feels
//          cramped while walking past walls.
//   600 -> sectors ~75 units, walls breathe.
#define map_Size           600

// Number of recursive BSP splits. The number of leaf sectors is 2^iteration.
//   iteration = 6  ->  64 sectors  ->  ~3 KB tree on AVR (fits in 8 KB SRAM).
//   iteration = 7  ->  128 sectors -> tight but doable.
//   iteration = 8  ->  256 sectors -> only with the slim 13-byte Container.
//
// `static constexpr` so every TU sees the value as a compile-time constant
// (no FP `pow` pulled in for View::rooms, no .data slot, no extern symbol).
static constexpr int iteration = 6;


/* ----- 3) Renderer ------------------------------------------------------- */

// Wall projection scale: lineHeight = WALL_PROJ_SCALE / depth.
// Rule of thumb:  WALL_PROJ_SCALE ~ (screenHeight / 2) * sectorSize.
// Lower values -> walls feel further away (less claustrophobic).
#define WALL_PROJ_SCALE    2400.0f

// Near plane in world units. Wall edges with depth below this are clipped
// (or skipped if both endpoints are behind it). Stops "infinite line height"
// at depth == 0 and removes the ghost-stripe artifact for walls behind the
// camera.
#define NEAR_PLANE_DEPTH   1.0f


/* ---- Floor / sky tinting ---- */
//
// 0 = solid black above and below walls (the original look).
// 1 = paint a sky tint on the upper half and a floor tint on the lower half.
//     Gives the world a horizon and makes rotation read more naturally,
//     because the eye has a stable up/down reference.
#define ENABLE_FLOOR_AND_SKY  1

// RGB565 colors. Tweak to taste. These defaults mimic a dusk Doom palette.
#define SKY_COLOR             0x29C7   // soft slate blue-gray
#define FLOOR_COLOR           0x4208   // muted dark brown


/* ----- 4) Gameplay ------------------------------------------------------- */

// Probability (0..100) that a leaf sector is randomly turned into a wall.
// 22 keeps the world open; raise toward 35-40 for a denser maze.
#define WALL_DENSITY_PERCENT  22

// Player collision radius in world units. The collision check tests the
// player center plus four points at +/- this radius along X and Y, so the
// player cannot touch a wall face. Combined with the perspective projection
// at NEAR_PLANE_DEPTH, this prevents the "stuck inside the wall" feel.
#define PLAYER_RADIUS         16

// Continuous turn / move rate while a hardware button is HELD (per frame).
// At ~30 FPS, HOLD_TURN_RATE = 0.06 rad gives ~100 deg/s, which feels
// natural without being twitchy.
#define HOLD_TURN_RATE        0.06f
#define HOLD_MOVE_RATE        3.0f

// Discrete step taken on a single Serial keystroke (w/a/s/d). Larger than
// the hold rate because each keystroke is a separate event - holding a key
// in the Serial Monitor only repeats at the OS auto-repeat rate.
#define SERIAL_TURN_STEP      0.18f
#define SERIAL_MOVE_STEP      9.0f

// Spawn point: world coordinates the player tries to spawn at. The actual
// spawn sector is the leaf containing this point; that sector is forced
// to be empty so every random wall is in front of the player.
#define SPAWN_SEED_X          (map_Size / 12)
#define SPAWN_SEED_Y          (map_Size / 12)

// Initial facing angle (radians). 7*PI/4 = -PI/4 in world convention points
// to +X, +Y, i.e. diagonally toward the opposite map corner.
#define SPAWN_ANGLE           (7.0f * PI / 4.0f)


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


#endif
