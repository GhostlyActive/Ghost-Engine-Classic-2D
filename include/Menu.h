#ifndef MENU_H
#define MENU_H

#include "Game.h"
#include "Raycasting_Map_Editor.h"
#include "Controls.h"


/* ============================================================================
 *  Menu.h
 *
 *  Two-entry boot menu drawn on the OLED:
 *      Position 1 = PLAY    -> Game_start (BSP engine)
 *      Position 2 = RAYCAST -> buildMapEditor (raycaster path)
 * ============================================================================ */


// Radius of the yellow cursor circle drawn next to the active item.
#define radius  5


class buildMenu
{
private:
    int position;

public:
    buildMenu();
    ~buildMenu();

    void ShowMenu(Adafruit_SSD1351 menu);
    void ShowCircle(Adafruit_SSD1351 menu, int x, int y);

    int  getPosition();
    void setPosition(int x);
};


#endif
