#ifndef RAYCASTING_Engine_H
#define RAYCASTING_Engine_H

/*
We are using Lode's Computer Graphics Tutorial. This version based on vectors and a camera.
https://lodev.org/cgtutor/raycasting.html

Also using the Adafruit SSD1351 OLED Driver.

We add a Y-Buffer and color-Buffer to speed up SPI Drawing on the screen
*/



#include "Raycasting_Settings.h"
#include "Display_Driver.h"
#include "Controlls.h"



void Load_Ray_Engine(Adafruit_SSD1351 tft, Player p);



#endif