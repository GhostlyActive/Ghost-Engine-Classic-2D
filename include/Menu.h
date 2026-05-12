#pragma once

#include "Game.h"
#include "Raycasting_Map_Editor.h"
#include "Controls.h"


/* ============================================================================
 *  Menu.h
 *
 *  Two-entry boot menu drawn on the OLED:
 *      MenuItem::Play    -> Game_start         (BSP engine)
 *      MenuItem::Raycast -> buildMapEditor     (raycaster path)
 * ============================================================================ */


// Radius of the yellow cursor circle drawn next to the active item.
constexpr int radius = 5;


// Which item the cursor is currently pointing at. Scoped enum so a stray
// `int` can't accidentally be passed where one of these is expected.
enum class MenuItem : uint8_t
{
    Play    = 0,
    Raycast = 1,
};


class Menu
{
public:
    Menu()  = default;
    ~Menu() = default;

    void show(Adafruit_SSD1351& tft);

private:
    void draw_cursor(Adafruit_SSD1351& tft, int x, int y) const;

    MenuItem position_ = MenuItem::Play;
};
