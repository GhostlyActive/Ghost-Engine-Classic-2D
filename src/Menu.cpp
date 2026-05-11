#include "Menu.h"


/* ============================================================================
 *  Menu.cpp
 *
 *  First screen shown on boot. Two entries:
 *      PLAY     -> Game_start (BSP engine with random walls)
 *      RAYCAST  -> buildMapEditor (map editor + Lode-style raycaster)
 *
 *  Selection: Forward/Backward to move the cursor, Action 1 to enter.
 * ============================================================================ */


// Y positions of the two menu items.
#define MENU_Y_PLAY     (radius * 3)
#define MENU_Y_RAYCAST  (screenHeight - radius * 5)


Menu::Menu()  { setPosition(1); }
Menu::~Menu() {}


void Menu::ShowMenu(Adafruit_SSD1351& menu)
{
    menu.fillScreen(BLACK);
    Btn_setup();

    // Initial cursor position.
    ShowCircle(menu, 5, MENU_Y_PLAY + radius);

    for (;;)
    {
        Serial_poll();

        menu.setTextColor(YELLOW);
        menu.setTextSize(2);

        menu.setCursor(5 + 20, MENU_Y_PLAY);    menu.println("PLAY");
        menu.setCursor(5 + 20, MENU_Y_RAYCAST); menu.println("RAYCAST");

        btn_Bounce_F.update();
        if (btn_Bounce_F.fell() || Serial_F())
        {
            setPosition(1);
            menu.fillScreen(BLACK);
            ShowCircle(menu, 5, MENU_Y_PLAY + radius);
        }

        btn_Bounce_B.update();
        if (btn_Bounce_B.fell() || Serial_B())
        {
            setPosition(2);
            menu.fillScreen(BLACK);
            ShowCircle(menu, 5, MENU_Y_RAYCAST + radius);
        }

        btn_Bounce_A1.update();
        if (btn_Bounce_A1.fell() || Serial_A1())
        {
            if (getPosition() == 1)
            {
                Game_start(menu);
            }
            else if (getPosition() == 2)
            {
                buildMapEditor buildMap;
                buildMap.Load_Map_Editor(menu);
            }
        }
    }
}


// Yellow selection circle rendered next to the active menu item.
void Menu::ShowCircle(Adafruit_SSD1351& menu, int x, int y)
{
    menu.setTextColor(YELLOW);
    menu.fillCircle(x, y, radius, YELLOW);
}


int  Menu::getPosition()        { return position; }
void Menu::setPosition(int x)   { position = x;    }
