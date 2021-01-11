#include "BSP_Engine.h"



struct Tree* newLeaf(struct Container leaf) 
{ 
  // Allocate memory for new node  
  struct Tree* Tree = (struct Tree*)malloc(sizeof(struct Tree)); 
  
  // Assign data to this node 
  Tree->leaf = leaf; 
  
  // Initialize left and right children as NULL 
  Tree->lchild = NULL; 
  Tree->rchild = NULL; 
  return(Tree); 
} 


struct Array2 geo_split(struct Container* container){

  struct Array2 two;

//vertical
  int direct = random(0,2);


   if(direct == 0) {

   two.array[0].x = container->x; two.array[0].y = container->y;
   //two.array[0].w = random(1, container->w); two.array[0].h = container->h;
   two.array[0].w = container->w/2; two.array[0].h = container->h;


   two.array[1].x = container->x + two.array[0].w; two.array[1].y = container->y;
   two.array[1].w = container->w - two.array[0].w; two.array[1].h = container->h;
   


// damit das mittig ist

            float r1_w_ratio = (float)two.array[0].w / (float)two.array[0].h;
            float r2_w_ratio = (float)two.array[1].w / (float)two.array[1].h;
           

          

            if (r1_w_ratio < 0.5 || r2_w_ratio < 0.5) {

         return geo_split(container);
            }

 }
//horizotal
 else {
   two.array[0].x = container->x; two.array[0].y = container->y;
   two.array[0].w = container->w; two.array[0].h = container->h/2 ; //random(1, container->h);

   two.array[1].x = container->x; two.array[1].y = container->y + two.array[0].h;
   two.array[1].w = container->w ; two.array[1].h = container->h - two.array[0].h;



            float r1_h_ratio = (float)two.array[0].h / (float)two.array[0].w;
            float r2_h_ratio = (float)two.array[1].h / (float)two.array[1].w;

            if (r1_h_ratio < 1 || r2_h_ratio < 1) {
             return geo_split(container);
            }

 }

return two;
}


struct Tree* split_container(struct Container container, int iter)
{

struct Tree* root;
root = newLeaf(container);


if(iter != 0) {

struct Array2 sr = geo_split(&container);


root->lchild = split_container(sr.array[0], iter-1);
root->rchild = split_container(sr.array[1], iter-1);


}

return root;

}


float VectorDegree(int &x_wall, int &y_wall, int &x2_wall, int &y2_wall) {   


//double abcde = atan((y2_wall-y_wall)/(x2_wall-x_wall)) * 180 / PI;
float angle = atan2(-y2_wall+y_wall, x2_wall-x_wall) * 180 / PI;


// keep between 0 - 360;
angle = angle + ceil( -angle / 360 ) * 360;

return angle;

 } 






struct Tree* printLeafNodes(struct Tree* root, struct Tree* order, int* step) 
{ 

    // if node is null, return 
    if (!root) 

        return order;
      
    // if node is leaf node, print its data     
    if (!root->lchild && !root->rchild) 
    { 
      root->leaf.order = *step;



      int xl = root->leaf.x - (root->leaf.w/2); int yl = root->leaf.y + (root->leaf.h /2);

      int xr = (root->leaf.x + root->leaf.w) + (root->leaf.w/2); int yr = root->leaf.y + (root->leaf.h /2);

      int xd = root->leaf.x + (root->leaf.w/2); int yd = root->leaf.y + (root->leaf.h /2);

      int xu = root->leaf.x + (root->leaf.w/2); int yu = root->leaf.y - (root->leaf.h/2);


      if(xl > 0 && xl< map_Size && yl > 0 && yl <map_Size)
      {
        if(getPosition(xl,yl ,root)->leaf.is_wall == true)
        {
            order[*step-1].leaf.on_left = true;
        }
      }

      if(xr > 0 && xr< map_Size && yr > 0 && yr <map_Size)
      {
        if(getPosition(xr,yr ,root)->leaf.is_wall == true)
        {
            order[*step-1].leaf.on_right = true;
        }
      }

      if(xd > 0 && xd< map_Size && yd > 0 && yd <map_Size)
      {
        if(getPosition(xd,yd ,root)->leaf.is_wall == true)
        {
            order[*step-1].leaf.on_down = true;
        }
      }

      if(xu > 0 && xu< map_Size && yu > 0 && yu <map_Size)
      {
        if(getPosition(xu,yu ,root)->leaf.is_wall == true)
        {
            order[*step-1].leaf.on_up = true;
        }
      }



     order[*step-1].leaf = root->leaf;

     order[*step-1].leaf.center.x = order[*step-1].leaf.x + (order[*step-1].leaf.w / 2);
     order[*step-1].leaf.center.y = order[*step-1].leaf.y + (order[*step-1].leaf.h / 2);

    //give edge x y values
    //left up corner
     order[*step-1].leaf.edge_lu.x =  order[*step-1].leaf.x;
     order[*step-1].leaf.edge_lu.y =  order[*step-1].leaf.y;

      //left down corner
      order[*step-1].leaf.edge_ld.x =  order[*step-1].leaf.x;
      order[*step-1].leaf.edge_ld.y =  (order[*step-1].leaf.y + order[*step-1].leaf.h) ;


      //right up corner
      order[*step-1].leaf.edge_ru.x =  (order[*step-1].leaf.x + order[*step-1].leaf.w);
      order[*step-1].leaf.edge_ru.y =  order[*step-1].leaf.y;

      //right down corner
      order[*step-1].leaf.edge_rd.x =  (order[*step-1].leaf.x + order[*step-1].leaf.w);
      order[*step-1].leaf.edge_rd.y =  (order[*step-1].leaf.y + order[*step-1].leaf.h);   

    (*step)++;

        return order;
    } 
  
    // if left child exists, check for leaf  
    // recursively 
    if (root->lchild) 
       printLeafNodes(root->lchild, order, step); 
          
    // if right child exists, check for leaf  
    // recursively 
    if (root->rchild) 
       printLeafNodes(root->rchild, order, step); 
  
}  


//search in which sector the player is standing and return  the node.
struct Tree* getPosition(int value_x, int value_y, struct Tree *node){

if(node->lchild== NULL && node->rchild == NULL){

return (node);

}

else if(node->lchild->leaf.x <= value_x && (node->lchild->leaf.x + node->lchild->leaf.w) >= value_x && node->lchild->leaf.y <= value_y  && (node->lchild->leaf.y + node->lchild->leaf.h) >= value_y)
{

return getPosition(value_x, value_y, node->lchild);

}


else if(node->rchild->leaf.x <= value_x && (node->rchild->leaf.x + node->rchild->leaf.w) >= value_x && node->rchild->leaf.y <= value_y  && (node->rchild->leaf.y + node->rchild->leaf.h) >= value_y)
{
return getPosition(value_x, value_y, node->rchild);

}


return NULL;

}




View Save_BSP_Engine(){

View v;
struct Tree* trees = new struct Tree[v.rooms];

struct Container main_container;


main_container.x = 0;
main_container.y = 0;
main_container.w = map_Size;
main_container.h = map_Size;


v.Container_map = split_container(main_container,iteration);

int zahl = 1;

v.test = printLeafNodes(v.Container_map, trees, &zahl);

return v;

}





//***************************DRAWING*******************************


/*
FILLSEGMENT()
* this function takes 2 distances and 2 positions on the x_position screen and interpolate it. 
* if dist_1 is bigger than dist_2 -> step in the for loop will be negativ -> lineHeight will be bigger from x_pos_1 to x_pos_2.
* x_Buffer is the clipping buffer for not overdrawing behind sector walls
* y_Buffer and colorBuffer save lineHeight and color from the last frame. When something changes -> just drawing and deleting changed pixels instead drawing every vertical
*/

void FillSegment(float dist_1, float dist_2, int x_pos_1, int x_pos_2, uint16_t color, Adafruit_SSD1351 tft, BSP_Player P, View V)
{

  float dist_diff = abs(dist_1 - dist_2); // delta distance
  int pos_diff = abs(x_pos_1 - x_pos_2); // delta x

  int lineHeight;                      // height on the screen. max. = screenHeight
  float step = dist_diff/pos_diff;     //interpolating step
  float for_step = 0;

  if(dist_1 < dist_2){                  
  
   step = -step;

  }

  // loop for interpolating between x_pos_1 and x_pos_2 also improve spi speed by only drawing changed pixels instead every whole vertical line
  for(int x = x_pos_1; x <= x_pos_2; x++) 
  {

     //lineHeight: between 0 - screenHeight
    lineHeight = (screenHeight-screenStatistics)- ((screenHeight-screenStatistics) *(dist_1 + for_step)/map_Size);   
    
    if(lineHeight <= 0)
    {
      lineHeight = 0;
    }

    // when [x] in clipping buffer is not used
    if(xBuffer[x] == false)
    {

      // if lineHeight saved before is not the same height
      if(yBuffer[x] != lineHeight)
      {

        //if Same color like before
        if(colorBuffer[x] == color)
        {
          int differenz = yBuffer[x]-lineHeight;
          int one = ((screenHeight-screenStatistics)- yBuffer[x])/2;
          int two = ((screenHeight-screenStatistics)-lineHeight)/2;

          //new line is getting smaller. Delete just the unwanted lines above and below the Wall.
          if(differenz > 0)
          {
            tft.drawFastVLine(x, one, two-one, BLACK);                  //above 
            tft.drawFastVLine(x, (screenHeight-screenStatistics)-two, two-one, BLACK);      //below
          }

          // new line is getting bigger. add just the lines above and below symetrical
          else
          {
            tft.drawFastVLine(x, two, one-two, color);
            int begin_y = (screenHeight-screenStatistics)-two-(one-two)-1;
            tft.drawFastVLine(x, begin_y, one-two, color);
          }
        }

        //if another color and not the same lineHeight
        else
        {
          int start_2 = ((screenHeight-screenStatistics)-lineHeight)/2;
          tft.drawFastVLine(x,0,(screenHeight-screenStatistics),BLACK);
          tft.drawFastVLine(x,start_2,lineHeight, color);
        }
      }

      // if same lineHeight but color changes
       else
      {
        if(colorBuffer[x] != color)
        {
          int start_3 = ((screenHeight-screenStatistics)-lineHeight)/2;
          tft.drawFastVLine(x,start_3,lineHeight,color);
        }
      }

      // save actual lineHeight and color in yBuffer and colorBuffer
      yBuffer[x] = lineHeight;
      colorBuffer[x] = color;

      // if vertical line is drawn -> save in the clipping buffer for not overdrawing at another sector
      
      xBuffer[x]= true;
    }

      // interpolating step 
      for_step = for_step - step;
  }

}




/*
DRAWSEGMENT()
* this function takes 2 edges coordinates 
* angle_1, angle_2 (worldangle from player to edges) are always between 0-360°
* it calculate distance from player to edges
* when comparing the angles with player´s left and right max. Angle we can say whether a edge is in the FOV and where it is on x_screen position 
* we are using virtual angle to ensure error-free calculation. Because angle_1 is always bigger than angle_2. it would be a problem when player facing 360° or 0°
* it calls the fillSegment function, which is drawing on the screen
*/

void DrawSegment(xy* edge1, xy* edge2, uint16_t color, Adafruit_SSD1351 tft, BSP_Player P, View V)
{

  float angle_1 = VectorDegree( P.player_px, P.player_py, edge1->x, edge1->y);
  float angle_2 = VectorDegree(P.player_px, P.player_py, edge2->x, edge2->y); 


  float backup_Angle_1 = angle_1;
  float backup_Angle_2 = angle_2;


  float diff_winkel =  PI-(V.playerAngle); // 180 - 270  //unten statt backup +diffwinkel-PI******

  float virtual_angle = V.playerAngle + diff_winkel;   //immer 180 grad


  angle_1 = angle_1 + (diff_winkel*180/PI);
  angle_2 = angle_2 + (diff_winkel*180/PI);

  //always between 0 - 360
  angle_1 = fmodf(angle_1, 360);    
      if (angle_1< 0)
          angle_1 += 360;

          angle_2 = fmodf(angle_2, 360);
      if (angle_2< 0)
          angle_2 += 360;


  V.playerAngle_left = (virtual_angle + (PI/4)) * 180/PI;
  V.playerAngle_right = (virtual_angle - (PI/4)) * 180/PI;


  // pos_1 is the first and left point of the x_screen_position, pos_2 is right point_position of the x_screen
  int pos_1_x;
  int pos_2_x;


// angle_1 is in the FOV
if(angle_1 <= V.playerAngle_left && angle_1 >= V.playerAngle_right )
{

  // 0 - screenWidth  depends on (playerAngle_left - angle_1)
  pos_1_x = (screenWidth-1) * (V.playerAngle_left-angle_1 )/ (V.FOV*180/PI);  // position on the screen from 0 - screenWidth

  float Distance_1;


  int VectorX_1 = edge1->x - P.player_px; // vector_x
  int VectorY_1 = edge1->y - P.player_py; // vector_y


  Distance_1 = sqrt(VectorX_1*VectorX_1 + (VectorY_1*VectorY_1))*cos(V.playerAngle-(backup_Angle_1*PI/180));  // lenght of the vector


  // angle_2 is also in the FOV
  if(angle_2 >= V.playerAngle_right){

    pos_2_x = (screenWidth-1) * (V.playerAngle_left-angle_2 )/ (V.FOV*180/PI);

    float Distance_2;


    int VectorX_2 = edge2->x - P.player_px;
    int VectorY_2 =  edge2->y - P.player_py;


    Distance_2 = sqrt(VectorX_2*VectorX_2 + (VectorY_2*VectorY_2))*cos(V.playerAngle-(backup_Angle_2*PI/180)); // länge des Vektors  //*************wichtig


    FillSegment(Distance_1, Distance_2, pos_1_x, pos_2_x, color, tft, P, V);

 
  }

  // else not in the FOV
  if(angle_2 < V.playerAngle_right){

    
    float Distance_2;

    // right outside from screen -> pos_2_x set on screenwidth
    pos_2_x = (screenWidth-1);



    int VectorX_2 = edge2->x - P.player_px;
    int VectorY_2 =  edge2->y - P.player_py;


    Distance_2 = abs(sqrt(VectorX_2*VectorX_2 + (VectorY_2*VectorY_2))*cos(V.playerAngle-(backup_Angle_2*PI/180))); // länge des Vektors  //*************wichtig



    FillSegment(Distance_1, Distance_2, pos_1_x, pos_2_x, color, tft, P, V);


  }
}


// angle_ 1 is left outside from playerAngle_left 
else if(angle_1 > V.playerAngle_left )
{

    // left outside from screen -> set to 0
    pos_1_x = 0;
    float Distance_1;



    int VectorX_1 = edge1->x - P.player_px;
    int VectorY_1 =  edge1->y - P.player_py;



    Distance_1 = abs(sqrt(VectorX_1*VectorX_1 + (VectorY_1*VectorY_1))*cos(V.playerAngle-(backup_Angle_1*PI/180)));  // länge des Vektors

  // angle_2 in the FOV
  if(angle_2 >= V.playerAngle_right){

    float Distance_2;

    pos_2_x = (screenWidth-1) * (V.playerAngle_left-angle_2 )/ (V.FOV*180/PI);

     

    int VectorX_2 = edge2->x - P.player_px;
    int VectorY_2 =  edge2->y - P.player_py;


    Distance_2 = sqrt(VectorX_2*VectorX_2 + (VectorY_2*VectorY_2))*cos(V.playerAngle-(backup_Angle_2*PI/180)); // länge des Vektors  //*************wichtig


    FillSegment(Distance_1, Distance_2, pos_1_x, pos_2_x, color, tft, P, V);


  }
    // angle_2 right outside from playerAngle_right
    if(angle_2 < V.playerAngle_right){

       float Distance_2;


    pos_2_x = (screenWidth-1);// * (playerAngle_left-angle_2 )/ (FOV*180/PI);


    int VectorX_2 = edge2->x - P.player_px;
    int VectorY_2 =  edge2->y - P.player_py;


    Distance_2 = sqrt(VectorX_2*VectorX_2 + (VectorY_2*VectorY_2))*cos(V.playerAngle-(backup_Angle_2*PI/180)); // länge des Vektors  //*************wichtig


    FillSegment(Distance_1, Distance_2, pos_1_x, pos_2_x, color, tft, P, V);

  }
 } 
}





/*
DRAWSEGMENT()
* this function takes Binary tree 
* checks whether this node includes a wall
* if: check which sides of a square are relevant from the player position
* this edges are going to the function DrawSegment
*/

void DrawWall(struct Tree* wall, Adafruit_SSD1351 tft, BSP_Player P, View V)
{

  //it`s a wall
  if(wall->leaf.is_wall == true)
  {
  
    if( P.player_px <= wall->leaf.x)
    {

      //position:      [WAll]
      //           *P

      if( P.player_py > (wall->leaf.y + wall->leaf.h))
      {

        if(wall->leaf.on_left == false)
          {
            DrawSegment(&wall->leaf.edge_lu, &wall->leaf.edge_ld, wall->leaf.color, tft, P, V);
          }

        if(wall->leaf.on_down == false)
        {
            DrawSegment(&wall->leaf.edge_ld, &wall->leaf.edge_rd, wall->leaf.color, tft, P, V);
        }
          
      }

      //           *P
      //position:      [Wall]
              
      else if( P.player_py < wall->leaf.y )
      {

         if(wall->leaf.on_left == false)
         {
            DrawSegment(&wall->leaf.edge_lu, &wall->leaf.edge_ld, wall->leaf.color, tft, P, V);
         }
         if(wall->leaf.on_up == false)
         {
          DrawSegment(&wall->leaf.edge_ru, &wall->leaf.edge_lu, wall->leaf.color, tft, P, V);
         }
        
      }

      //position:    *P  [Wall]
      else 
      {    

       DrawSegment(&wall->leaf.edge_lu, &wall->leaf.edge_ld, wall->leaf.color, tft, P, V);

      }

    }


    else if( P.player_px> (wall->leaf.x + wall->leaf.w))
    {

      //position:      [WAll]
      //                        *P
      if( P.player_py > (wall->leaf.y + wall->leaf.h))
      {

         if(wall->leaf.on_right == false)
         {
          DrawSegment(&wall->leaf.edge_rd, &wall->leaf.edge_ru, wall->leaf.color, tft, P, V);
         }

         if(wall->leaf.on_down == false)
         {
          DrawSegment(&wall->leaf.edge_ld, &wall->leaf.edge_rd,  wall->leaf.color, tft, P, V);
         }
      }

      //                          *P
      //position:      [WAll]

      else if( P.player_py <  wall->leaf.y)
      {

          if(wall->leaf.on_right == false)
          {
          DrawSegment(&wall->leaf.edge_rd, &wall->leaf.edge_ru, wall->leaf.color, tft, P, V);
          }

          if(wall->leaf.on_up == false)
          {
          DrawSegment(&wall->leaf.edge_ru, &wall->leaf.edge_lu,  wall->leaf.color, tft, P, V);
          }
      }

      //position:      [WAll]    *P

      else 
      {

       DrawSegment(&wall->leaf.edge_rd, &wall->leaf.edge_ru, wall->leaf.color, tft, P, V);
       
      }

    }


    else if( P.player_px >= wall->leaf.x &&  P.player_px <= (wall->leaf.x + wall->leaf.w))
    {

      //                 *P
      //position:      [WAll]
      if( P.player_py <= wall->leaf.y)
      {
        
       DrawSegment(&wall->leaf.edge_ru, &wall->leaf.edge_lu,  wall->leaf.color, tft, P, V);

      }
   
      //position:      [WAll]
      //                 *P

      else if( P.player_py >= (wall->leaf.y + wall->leaf.h)) 
      {
        
        DrawSegment(&wall->leaf.edge_ld, &wall->leaf.edge_rd, wall->leaf.color, tft, P, V);
      
      }
    }
  }

  // its not a wall
  else if(wall->leaf.is_wall == false)
  {

  }
}


/*
FIELDOFVIEW2ANGLE()
* its called every frame 
* update playerAngle_left and playerAngle_right
* playerAngle -> 0 - 360
*/

void FieldOfView2Angle(View V)
{
  // playerAngle always between 0 - 360
  V.playerAngle = fmodf(V.playerAngle, 2*PI);

    if (V.playerAngle < 0)
        V.playerAngle+= 2*PI;
  
    
  V.playerAngle_left = (V.playerAngle + (V.FOV / 2) ) * 180 / PI;     // playerAngle_left = playerAngle + (FOV/2)
  V.playerAngle_right = (V.playerAngle - (V.FOV / 2) ) * 180 / PI;   // playerAngle_right = playerAngle_left - FOV; 
    
}



//from nearest to fartest wall, abh. from postion of the player
 void PrintCloseToFar(struct Tree* AT, int start, Adafruit_SSD1351 tft, BSP_Player P, View V ){

   int index = start-1;

   int a =  V.rooms;

   int j = index +1;

//erste Schritt nach rechts
   while(j < a || index >= 0)
   {

    if (index >= 0){

    DrawWall(&AT[index], tft, P, V);
}

    if (j < a) 
    {
            
        DrawWall(&AT[j], tft, P, V);
    }

    index--;
    j++; 
    
   }
}


















