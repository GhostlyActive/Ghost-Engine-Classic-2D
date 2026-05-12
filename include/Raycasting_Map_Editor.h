#pragma once
#include "Display_Driver.h"
#include "Raycasting_Engine.h"
#include "Controls.h"


/* ============================================================================
 *  Raycasting_Map_Editor.h
 *
 *  Grid map editor for the RAYCAST path. The outer border is always wall
 *  (id 1); the inside cells can be toggled by the player. Action 1 cycles
 *  the cell color, Action 2 launches Load_Ray_Engine on the resulting map.
 * ============================================================================ */


class buildMapEditor
{
public:
    const int size_x = mapWidth;
    const int size_y = mapHeight;

    int position_x = size_x / 2;     // cursor cell (X)
    int position_y = size_y / 2;     // cursor cell (Y)

    void Load_Map_Editor(Adafruit_SSD1351& tft);
};


