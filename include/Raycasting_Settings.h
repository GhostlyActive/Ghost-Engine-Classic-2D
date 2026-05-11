#ifndef RAYCASTING_SETTINGS_H
#define RAYCASTING_SETTINGS_H


/* ============================================================================
 *  Raycasting_Settings.h
 *
 *  Settings for the Lode-style raycaster path (the RAYCAST menu entry).
 *  The BSP engine has its own settings in BSP_Settings.h - the two paths
 *  intentionally don't share state.
 * ============================================================================ */


// OLED resolution. BSP_Settings.h defines the same two macros with the
// same values; if a TU includes both headers the preprocessor sees identical
// `#define`s and stays quiet. Kept duplicated so the raycaster path is
// self-contained (Raycasting_Engine.cpp does not include BSP_Settings.h).
#ifndef screenWidth
#define screenWidth   128
#endif
#ifndef screenHeight
#define screenHeight  128
#endif

#define mapWidth   24
#define mapHeight  24


// Live grid edited by Raycasting_Map_Editor, sampled by Raycasting_Engine.
//   0 = empty, 1..N = wall ids (see Raycasting_Engine.cpp color switch)
extern int worldMap[mapWidth][mapHeight];


// Player state for the raycaster. Position is in fractional cell units;
// (dirX, dirY) is the unit forward vector; (planeX, planeY) is the half
// of the 2D camera plane perpendicular to the forward vector - its length
// sets the FOV.
struct Player
{
    double posX     = 12,   posY   = 12;     // spawn at map center
    double dirX     = -1,   dirY   = 0;      // facing -X
    double planeX   = 0,    planeY = 0.66;   // ~66 deg FOV

    double moveSpeed = 0.25;
    double rotSpeed  = 0.2;
};


#endif
