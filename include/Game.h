#ifndef GAME_h
#define GAME_h

#include "BSP_Map_Editor.h"
#include "Menu.h"


//start bsp_map_editor
void Game_start(Adafruit_SSD1351 tft);


// real game starts. Rendering Engine starts and draw textures and and load game logic. working controls.
void Game_loop(Adafruit_SSD1351 tft, BSP_Player P, View v);



/********************************************TEXT WIN/LOSE***********************************************
************************************************************************************************/

// win and lose text -> and goto menu scene
void win(Adafruit_SSD1351 menu);
void lose(Adafruit_SSD1351 menu);



/********************************************GAMEPLAY ELEMENTS************************************
************************************************************************************************/


// this function update wheter left,right, up, down is a near wall
void game_Near_Walls(struct Tree* AT, View V, int mode);

// working compass. target is always north in the compass
void compass(Adafruit_SSD1351 menu, BSP_Player P, View V);



/********************************************TEXTURE********************************************
************************************************************************************************/

// Textures are made with GIMP. Than convertet into this form
// converter: http://javl.github.io/image2cpp/


// Hand texture (Bitmap)
extern unsigned char Hand[];


// status bar texture
extern unsigned char stats[];


#endif