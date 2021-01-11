#ifndef BSP_SETTINGS_h
#define BSP_SETTINGS_h

#include <stdint.h>

// screen pixels
#define screenWidth 128
#define screenHeight 128

// choose a unit for the map Size. Map is square
#define map_Size 400


//define number of pixels for certain playful elements. 
#define screenStatistics 20


//define BSP division. iteration^2 = rooms
extern int iteration;

//clipping Buffer
extern bool xBuffer[screenWidth];

extern int yBuffer[screenWidth];

// Color Buffer for all columns
extern uint16_t colorBuffer[screenWidth];



//Player stats for position on the map. Define moving and turning speed
struct BSP_Player 
{
  int player_px = 100;  
  int player_py = 100;   

  float playerTurn = 0.2;
  float playerMove = 8.0;    


  // game specific variables
  //health bar
  int health = 100;

  // when standing on this sector -> player wins
  int target_sector;
  int target_x;
  int target_y;

  

};


#endif