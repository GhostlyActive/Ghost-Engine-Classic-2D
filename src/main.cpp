//  1.)  showMenu
//  1.1) "play" ->BSP_Map_Editor -> BSP_Engine 
//  1.2) "Settings" -> BSP 
//  1.3) "Raycast" ->  Raycasting_Map_Editor -> Raycasting_Engine


#include "Display_Driver.h"
#include "Menu.h"


//OLED DISPLAY -> Constructor for Hardware SPI. Much faster than Software SPI 
Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);


//Class Object for graphic Menu
buildMenu build;



void setup() 
{

  //configuration of the RGB OLED Display
  tft.begin();

  //choose starting color 
  tft.fillScreen(BLACK);

  tft.setSPISpeed(10000000);
  
  Serial.begin(225000);

  //load Menu
  build.ShowMenu(tft);

} 


void loop()
{

}


