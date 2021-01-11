#ifndef BSP_MAP_EDITOR_H
#define BSP_MAP_EDITOR_H

#include "BSP_Engine.h"

#include "Game.h"

#include "Controlls.h"

/*
Map Editor for setting walls with color

When finished BSP-Engine and game will start.
*/

class BSPEditor
{ 

public:

const int size = map_Size;


 int position_x =  size/2;
 int position_y =  size/2;

public:
//BSPEditor();
//~dBSPEditor();

// load the Map_editor for setting walls with different colors
void Load_BSP_Editor(Adafruit_SSD1351 tft, struct Tree* AT, View V);

// set wall to actual position 
void Set_Wall(struct Tree* AT, BSP_Player player, View V, int mode);

// this function checks whether there is a wall close to the actual wall in an array
void Near_Walls(struct Tree* AT, View V);


void TextMode(Adafruit_SSD1351 menu, int mode);

};



#endif