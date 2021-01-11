/*
Adafruit-ssd1341 Library for the RGB OLED Screen. Written by Limor Fried/Ladyada for Adafruit Industries.
https://github.com/adafruit/Adafruit-SSD1351-library
Adafruit GFX graphics core library. Common set of graphics primitives. 
https://github.com/adafruit/Adafruit-GFX-Library

For this project we are using the Adafruit constructor for Hardware SPI Init, drawFastVLine for vertical Lines and fillScreen function.

*/

#ifndef DISPLAY_DRIVER_h
#define DISPLAY_DRIVER_h

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>

//Display Resolution
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 

//pinout for OLED Driver (SCLK_PIN and MOSI_PIN are used in Hardware SPI on Arduino Due -> no digital pins needed)
#define SCLK_PIN 13
#define MOSI_PIN 11
#define DC_PIN   8
#define CS_PIN   10
#define RST_PIN  9

#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF


#endif