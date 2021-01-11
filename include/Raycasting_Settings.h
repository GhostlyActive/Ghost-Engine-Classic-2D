#ifndef RAYCASTING_SETTINGS_h
#define RAYCASTING_SETTINGS_h

//x,y resolution of the OLED Display and Mapsize
#define screenWidth 128  
#define screenHeight 128
#define mapWidth 24
#define mapHeight 24


//declare array as extern
extern int worldMap[mapWidth][mapHeight];


//Player position and abilities
struct Player {
  double posX = 12, posY = 12;        //x and y start position
  double dirX = -1, dirY = 0;         //initial direction vector
  double planeX = 0, planeY = 0.66;   //the 2d raycaster version of camera plane   planeY -> FOV
 
  double moveSpeed = 0.25;            //moving speed
  double rotSpeed = 0.2;              // rotating speed
};




#endif