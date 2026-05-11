#include "BSP_Settings.h"


/* ============================================================================
 *  BSP_Settings.cpp - storage for the per-column run-time buffers.
 *
 *  The BSP depth (`iteration`) is now a compile-time `static constexpr`
 *  in BSP_Settings.h, so no definition is needed here.
 * ============================================================================ */


// Per-column buffers used by the renderer (one entry per screen column).
//
//   xBuffer     - was column drawn this frame? (per-frame occlusion list)
//   yBuffer     - last drawn line height (for delta redraw)
//   colorBuffer - last drawn color       (for delta redraw)

bool      xBuffer[screenWidth]     = {0};
int       yBuffer[screenWidth]     = {0};
uint16_t  colorBuffer[screenWidth] = {0};
