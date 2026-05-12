#pragma once
#include "Raycasting_Settings.h"
#include "Display_Driver.h"
#include "Controls.h"


/* ============================================================================
 *  Raycasting_Engine.h
 *
 *  Lode-style vector-camera raycaster (https://lodev.org/cgtutor/raycasting.html).
 *  One DDA ray per screen column on the 24x24 grid in Raycasting_Settings.h.
 *  Y- and color-buffers speed up SPI drawing the same way the BSP path
 *  does it - only the columns whose height or color changed are re-pushed.
 * ============================================================================ */


// Enter the first-person raycaster on the current worldMap. Never returns.
void Load_Ray_Engine(Adafruit_SSD1351& tft, Player p);


