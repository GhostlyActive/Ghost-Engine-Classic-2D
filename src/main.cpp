#include "Display_Driver.h"
#include "Menu.h"
#include "Controls.h"


/* ============================================================================
 *  main.cpp - boot entry point.
 *
 *  Boot flow:
 *      setup()                    Display + Serial + RNG seed
 *        -> menu.show(tft)
 *          -> Game_start(tft)                (PLAY)
 *          -> buildMapEditor.Load_Map_Editor (RAYCAST)
 *
 *  Both ShowMenu and Game_loop are infinite for-loops, so loop() never runs.
 * ============================================================================ */


// Hardware-SPI constructor. Much faster than software SPI on AVR.
Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI,
                                        CS_PIN, DC_PIN, RST_PIN);

// Persistent menu instance.
Menu menu;


void setup()
{
    tft.begin();
    tft.fillScreen(BLACK);
    tft.setSPISpeed(10000000);

    // Must match monitor_speed in platformio.ini.
    Serial.begin(115200);

    // Seed the RNG from a floating analog pin so every boot picks a
    // different BSP layout and wall pattern.
    randomSeed(analogRead(A0));

    Serial_help();

    menu.show(tft);
}


void loop()
{
    // Unreachable: ShowMenu / Game_loop never return.
}
