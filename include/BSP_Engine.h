#ifndef BSP_Engine_H
#define BSP_Engine_H

#include "BSP_Settings.h"
#include "Display_Driver.h"

/*
BSP mapping and building is based on this
https://eskerda.com/bsp-dungeon-generation/

-> Translated from Javascript to C

Ideas of Drawing and Rendering is inspired by Doom 1993

Also using the Adafruit SSD1351 OLED Driver.

*/



// this important struct defines number of room, angle, FOV of player and includes Container_map for all coordinates / nodes of sectors
struct View
{

  int rooms = pow(2,iteration);

  float playerAngle = PI/2;

  float FOV = (PI/2);

  float playerAngle_left = (playerAngle + (FOV / 2) ) * 180 / PI;

  float playerAngle_right = playerAngle_left - 90; //(playerAngle - (FOV / 2) ) * 180 / PI;


  struct Tree* test;

  struct Tree* Container_map;

};


// struct for saving x and y coordinates in one place
struct xy
{

int x;
int y;

};


// its the node for all sectors. This includes coordinates, color, edges coordinates, neighbor wall exist bool 
struct Container
{

  // coordinates
  int x;
  int y;
  int w;
  int h; 
  int order;
  uint16_t color = BLACK;
  bool is_wall = false;
  bool is_enemy = false;

  struct xy center;

  struct xy edge_lu;
  struct xy edge_ld;
  struct xy edge_ru;
  struct xy edge_rd;

  // neighbour walls?
  bool on_left = false;
  bool on_right = false;
  bool on_up = false;
  bool on_down = false;


};


//want to return two array values from a function
struct Array2 
{
    struct Container array[2];
};

// building a binary tree. 
struct Tree  
{ 
  struct Container leaf;
  struct Tree *lchild ; 
  struct Tree *rchild ; 

};

// This are function for building a binary tree and splitting the map with a certain interaton number.

struct Tree* newLeaf(struct Container leaf);

struct Array2 geo_split(struct Container* container);

struct Tree* split_container(struct Container container, int iter);

float VectorDegree(int &x_wall, int &y_wall, int &x2_wall, int &y2_wall);

struct Tree* printLeafNodes(struct Tree* root, struct Tree* order, int* step);

struct Tree* getPosition(int value_x, int value_y, struct Tree *node);

View Save_BSP_Engine();


//*********************DRAWING***********************

void FillSegment(float dist_1, float dist_2, int x_pos_1, int x_pos2, uint16_t color, Adafruit_SSD1351 tft, BSP_Player P, View V);

void DrawSegment(xy* edge1, xy* edge2, uint16_t color, Adafruit_SSD1351 tft, BSP_Player P, View V);

void DrawWall(struct Tree* wall, Adafruit_SSD1351 tft, BSP_Player P, View V);

void FieldOfView2Angle(View V);

void PrintCloseToFar(struct Tree* AT, int start, Adafruit_SSD1351 tft, BSP_Player P, View V );

void Load_BSP_Engine(Adafruit_SSD1351 tft, BSP_Player P, View V);





#endif