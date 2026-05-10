#include "BSP_Settings.h"


/* ============================================================================
 *  BSP_Settings.cpp - storage for the run-time buffers and the BSP depth.
 * ============================================================================ */


// Per-column buffers used by the renderer (one entry per screen column).
//
//   xBuffer     - was column drawn this frame? (per-frame occlusion list)
//   yBuffer     - last drawn line height (for delta redraw)
//   colorBuffer - last drawn color       (for delta redraw)

bool      xBuffer[screenWidth]     = {0};
int       yBuffer[screenWidth]     = {0};
uint16_t  colorBuffer[screenWidth] = {0};


// Number of recursive BSP splits. 2^iteration sectors total.
//
// Mega 2560 has 8 KB SRAM. With the slim 13-byte Container struct,
// iteration = 6 (64 sectors, ~3 KB tree) leaves plenty of room for the
// rest of the runtime. Bumping to 7 (128 sectors) still fits but gets tight.
int iteration = 6;
