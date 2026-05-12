#pragma once
#include "BSP_Engine.h"


/* ============================================================================
 *  Game.h
 *
 *  The "PLAY" entry point of the menu. Generates a BSP world, scatters
 *  random colored walls, places the player at a corner sector facing
 *  inward, and runs the render/movement loop.
 *
 *  No map editor, no enemies, no goal - just walk around the procedural
 *  geometry.
 * ============================================================================ */


// Build the BSP world, scatter walls, and enter Game_loop. Returns to the
// caller (the menu) only on RAM allocation failure.
void Game_start(Adafruit_SSD1351& tft);


// Per-frame render + input loop. Owns the player position and view angle.
// Movement uses sector-based collision with PLAYER_RADIUS and slides along
// walls.
void Game_loop(Adafruit_SSD1351& tft, BSP_Player P, View V);


