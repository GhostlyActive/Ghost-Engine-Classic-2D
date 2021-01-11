// 2d Map on OLED SCREEN which can be modified. 
// outer borders are always 1. Player can change everything inside of this walls.
//Color change by clicking on  action button 

#ifndef RAYCASTING_MAP_EDITOR_H
#define RAYCASTING_MAP_EDITOR_H

#include "Display_Driver.h"
#include "Raycasting_Engine.h"
#include "Controlls.h"


class buildMapEditor { 

public:

const int size_x = mapWidth;
const int size_y = mapHeight;

int position_x = size_x/2;
int position_y = size_y/2;

public:
//buildMapEditor();
//~buildMapEditor();
void Load_Map_Editor(Adafruit_SSD1351 tft);


};

#endif