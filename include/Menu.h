//First thing which is seen on the OLED Display
//ShowMenu -> start , settings , raycast 
//ShowCircle is the graphical position of the circle. 
//position 1 is start, position 2 is settings and position 3 is raycast

#ifndef MENU_H
#define MENU_H

#include "Raycasting_Map_Editor.h"
#include "Controlls.h"

//#include "BSP_Map_Editor.h"

#include "Game.h"


#define radius  5


class buildMenu { 

private:

int position;

public:
buildMenu();
~buildMenu();
void ShowMenu(Adafruit_SSD1351);
void ShowCircle(Adafruit_SSD1351, int x, int y);

int getPosition();
void setPosition(int x);

};



#endif