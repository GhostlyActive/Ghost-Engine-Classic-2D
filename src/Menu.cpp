#include "Menu.h"


/* ============================================================================
 *  Menu.cpp
 *
 *  First screen shown on boot. Two entries:
 *      Play    -> Game_start            (BSP engine + random walls)
 *      Raycast -> buildMapEditor        (Lode-style raycaster + map editor)
 *
 *  Forward / Backward move the cursor, Action 1 enters the selected mode.
 * ============================================================================ */


namespace {

// Y positions of the two menu items on screen.
constexpr int kPlayY    = radius * 3;
constexpr int kRaycastY = screenHeight - radius * 5;

constexpr int row_y(MenuItem item)
{
    return item == MenuItem::Play ? kPlayY : kRaycastY;
}

}  // namespace


void Menu::show(Adafruit_SSD1351& tft)
{
    tft.fillScreen(BLACK);
    Btn_setup();

    draw_cursor(tft, 5, row_y(position_) + radius);

    for (;;)
    {
        Serial_poll();

        tft.setTextColor(YELLOW);
        tft.setTextSize(2);

        tft.setCursor(5 + 20, kPlayY);    tft.println("PLAY");
        tft.setCursor(5 + 20, kRaycastY); tft.println("RAYCAST");

        btn_Bounce_F.update();
        if (btn_Bounce_F.fell() || Serial_F())
        {
            position_ = MenuItem::Play;
            tft.fillScreen(BLACK);
            draw_cursor(tft, 5, kPlayY + radius);
        }

        btn_Bounce_B.update();
        if (btn_Bounce_B.fell() || Serial_B())
        {
            position_ = MenuItem::Raycast;
            tft.fillScreen(BLACK);
            draw_cursor(tft, 5, kRaycastY + radius);
        }

        btn_Bounce_A1.update();
        if (btn_Bounce_A1.fell() || Serial_A1())
        {
            switch (position_)
            {
                case MenuItem::Play:
                    Game_start(tft);
                    break;

                case MenuItem::Raycast:
                {
                    buildMapEditor editor;
                    editor.Load_Map_Editor(tft);
                    break;
                }
            }
        }
    }
}


// Yellow selection circle rendered next to the active menu item.
void Menu::draw_cursor(Adafruit_SSD1351& tft, int x, int y) const
{
    tft.setTextColor(YELLOW);
    tft.fillCircle(x, y, radius, YELLOW);
}
