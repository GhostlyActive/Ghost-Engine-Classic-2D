#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <stdint.h>


/* ============================================================================
 *  Display_Driver.h
 *
 *  Hardware-SPI wiring for the 1.5" SSD1351 RGB OLED + RGB565 color names.
 *  All values are typed `constexpr`s rather than `#define`s so we get type
 *  safety and proper scoping. Uses the Adafruit GFX + SSD1351 libraries
 *  (pulled in via PlatformIO lib_deps).
 * ============================================================================ */


/* ----- Display resolution ----------------------------------------------- */

constexpr int SCREEN_WIDTH  = 128;
constexpr int SCREEN_HEIGHT = 128;


/* ----- OLED wiring on the Mega 2560 ------------------------------------- */
//
// SCLK and MOSI are routed through the AVR's hardware-SPI peripheral, so no
// extra digital pin work is needed for those.

constexpr uint8_t SCLK_PIN = 52;
constexpr uint8_t MOSI_PIN = 51;
constexpr uint8_t DC_PIN   = 9;
constexpr uint8_t CS_PIN   = 10;
constexpr uint8_t RST_PIN  = 8;


/* ----- RGB565 palette --------------------------------------------------- */

constexpr uint16_t BLACK   = 0x0000;
constexpr uint16_t BLUE    = 0x001F;
constexpr uint16_t RED     = 0xF800;
constexpr uint16_t GREEN   = 0x07E0;
constexpr uint16_t CYAN    = 0x07FF;
constexpr uint16_t MAGENTA = 0xF81F;
constexpr uint16_t YELLOW  = 0xFFE0;
constexpr uint16_t WHITE   = 0xFFFF;
