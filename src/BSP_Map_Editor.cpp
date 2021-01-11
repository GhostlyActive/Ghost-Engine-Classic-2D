#include "BSP_Map_Editor.h"


/*
Map Editor for setting walls with color

When finished -> BSP-Engine and game will start.
*/


void BSPEditor::Load_BSP_Editor(Adafruit_SSD1351 tft, struct Tree* AT, View V)
{
  //declare object for saving the coordinates of player. And for loading the Engine
  BSP_Player P;

  // setup Pinmodes for controll
  Btn_setup();

  P.player_px = AT[0].leaf.center.x;
  P.player_py = AT[0].leaf.center.y;

  //step is the lenght for moving left and right on the map editor
  int step =  abs(AT[0].leaf.center.y - AT[1].leaf.center.y);

 


  tft.fillScreen(BLACK);

  int mode = 0;

  // "set Wall" board
  tft.fillScreen(BLACK);
  tft.drawRect(0,0,128,128,BLUE);

  tft.setCursor(10, (screenHeight/2)-10);
  tft.setTextSize(2);
  tft.println("SET WALLS");
  tft.setTextSize(2);

  delay(1000);
  tft.fillScreen(BLACK);

  for(;;)
  {

    int index = 0;


    //maximum rooms
    int a = V.rooms;

    //draw all Rooms
     while(index < a )
    {

      int x = AT[index].leaf.x*(screenWidth-1)/map_Size;
      int y = AT[index].leaf.y*(screenWidth-1)/map_Size;
      int w = AT[index].leaf.w*(screenWidth-1)/map_Size;
      int h = AT[index].leaf.h*(screenWidth-1)/map_Size;

      if(AT[index].leaf.is_wall == true)
      {
        tft.fillRect(x,y,w,h,AT[index].leaf.color);
        tft.drawRect(x,y,w,h,BLUE);
      }

      else
      {

      tft.drawRect(x,y,w,h,BLUE);
      }

      if(AT[index].leaf.is_enemy == true){
         tft.fillRect(x,y,w,h,WHITE);

      }

      index++;
      
    }

    //draw moving lines on screen
    tft.drawFastVLine(P.player_px *(screenWidth-1)/map_Size ,0,SCREEN_HEIGHT,WHITE);
    tft.drawFastHLine(0,P.player_py *(screenWidth-1)/map_Size ,SCREEN_WIDTH,WHITE);


        /********************************************Controlls***********************************************
        ****************************************************************************************************/
        // action_2 (start)
        // up
        // down
        // right
        // left
        // action_1 (select)

        //start game or engine
        // button action_2
        btn_Bounce_A2.update();
		    if(btn_Bounce_A2.fell())
        {
            mode = mode+1;
            int position = getPosition(P.player_px, P.player_py,V.Container_map)->leaf.order;

            
            if(mode == 1)
            {

              TextMode(tft,1);
            }


            if(mode == 2)
            {

              TextMode(tft,2);
            }
            if(mode == 3)
            {


              if(AT[position-1].leaf.is_wall == false && AT[position-1].leaf.is_enemy == false)
              {
                 TextMode(tft,3);
                P.target_sector = position;
                P.target_x = AT[position-1].leaf.center.x;
                P.target_y = AT[position-1].leaf.center.y;

              }
              else
              {
                mode = 2;
                TextMode(tft, 42);
                
              }

            }

            if(mode == 4)
            {

              if(AT[position-1].leaf.is_wall == false && AT[position-1].leaf.is_enemy == false)
              {
                  
                Near_Walls(AT,V);
                tft.fillScreen(BLACK);

                Game_loop(tft, P, V);
              } 
              else
              {
                mode = 3;
                TextMode(tft, 42);
                
              }
            }

        }

        // go up
        btn_Bounce_F.update();
		    if(btn_Bounce_F.fell())  
        {  

          tft.fillScreen(BLACK);

          if(P.player_py > step) 
          {

            P.player_py = P.player_py - step;
          }

        }

        // go down
        btn_Bounce_B.update();
		    if(btn_Bounce_B.fell())
        {  

          tft.fillScreen(BLACK);

          if(P.player_py < map_Size-step) 
          {

            P.player_py = P.player_py + step;
          }
        }

        // go left
        btn_Bounce_L.update();
		    if(btn_Bounce_L.fell())
        {  

       tft.fillScreen(BLACK);

          if(P.player_px > step) 
          {

            P.player_px = P.player_px - step;
          }
        }

        // go right
        btn_Bounce_R.update();
		    if(btn_Bounce_R.fell())
        {  

          tft.fillScreen(BLACK);
        
          if(P.player_px < map_Size-step)
          {

            P.player_px = P.player_px + step;
          } 
        }


      // Button Action_1 
      btn_Bounce_A1.update();
		  if(btn_Bounce_A1.fell())
      {  
        if(mode == 0)
        {
          Set_Wall(AT, P, V, 0);
        }

        if(mode == 1)
        {
          Set_Wall(AT, P, V, 1);
        }

        tft.fillScreen(BLACK);
      }  
  }
}


void BSPEditor::TextMode(Adafruit_SSD1351 menu, int mode){

  
    menu.fillScreen(BLACK);
    
    menu.drawRect(0,0,128,128,BLUE);

    if(mode == 42)
    {
      menu.drawRect(0,0,128,128,RED);
    }


    menu.setCursor(10, (screenHeight/2)-10);
    menu.setTextSize(2);

    if(mode == 1){ menu.println("SET ENEMY");}
    if(mode == 2){ menu.println("AIM POINT");}
    if(mode == 3){ menu.println("SET SPAWN");}
    if(mode == 42){menu.println("  TAKEN");}


    delay(1000);
    menu.fillScreen(BLACK);
}





void BSPEditor::Set_Wall(struct Tree* AT, BSP_Player player, View V, int mode)
{

  for(int step = 0; step< V.rooms; step++)
  {

    if(AT[step].leaf.center.x == player.player_px && AT[step].leaf.center.y == player.player_py)
    {

      switch (AT[step].leaf.color)
      {
          case BLACK:
          if(mode == 0){ AT[step].leaf.color = RED; AT[step].leaf.is_wall = true;}
          if(mode == 1)
          { 
             AT[step].leaf.color = BLACK;
             AT[step].leaf.is_wall = false;
             if(AT[step].leaf.is_enemy == false){ AT[step].leaf.is_enemy = true;}
             else{AT[step].leaf.is_enemy = false;}
          }
          break;

          case RED:
          if(mode == 0){ AT[step].leaf.color = BLUE;}
          if(mode == 1){ AT[step].leaf.color = BLACK; AT[step].leaf.is_wall = false; AT[step].leaf.is_enemy = true;}
          break;

          case BLUE:
          if(mode == 0){ AT[step].leaf.color = YELLOW;}
          if(mode == 1){ AT[step].leaf.color = BLACK; AT[step].leaf.is_wall = false; AT[step].leaf.is_enemy = true;}
          break;

          case YELLOW:
          if(mode == 0){ AT[step].leaf.color = BLACK; AT[step].leaf.is_wall = false;}
          if(mode == 1){ AT[step].leaf.color = BLACK; AT[step].leaf.is_wall = false; AT[step].leaf.is_enemy = true;}
          break;

          default: 
          if(mode == 0){ AT[step].leaf.color = BLACK; AT[step].leaf.is_wall = false;}
          if(mode == 1){ AT[step].leaf.color = BLACK; AT[step].leaf.is_wall = false; AT[step].leaf.is_enemy = false;}
          break;
      }
    }
  }
}


void BSPEditor::Near_Walls(struct Tree* AT, View V)
{

  for(int x = 0; x<V.rooms; x++)
  {

    if(AT[x].leaf.is_wall == true)
    {

      // save coordinates in variables
      int AT_x = AT[x].leaf.x;
      int AT_y = AT[x].leaf.y;
      int AT_w = AT[x].leaf.w;
      int AT_h = AT[x].leaf.h;

      //this variable save coordinates of the close sectors
      int xl = AT_x  - (AT_w/2); int yl = AT_y + (AT_h /2);
      int xr = (AT_x  + AT_w) + (AT_w/2); int yr = AT_y + (AT_h /2);
      int xd = AT_x  + (AT_w/2); int yd = AT_y+ AT_h +(AT_h/2);
      int xu = AT_x  + (AT_w/2); int yu = AT_y - (AT_h/2);

      //check if left sector is a wall
      if(xl > 0 && xl< map_Size && yl > 0 && yl <map_Size)
      {
        int pos = getPosition(xl,yl ,V.Container_map)->leaf.order;
        if(AT[pos-1].leaf.is_wall == true)
        {
            AT[x].leaf.on_left = true;
        }
      }

      //check if right sector is a wall
      if(xr > 0 && xr< map_Size && yr > 0 && yr <map_Size)
      {

      int pos = getPosition(xr,yr ,V.Container_map)->leaf.order;
      if(AT[pos-1].leaf.is_wall == true)
        {
            AT[x].leaf.on_right= true;
        }
      }

      //check if down sector is a wall
      if(xd > 0 && xd< map_Size && yd > 0 && yd <map_Size)
      {

        int pos = getPosition(xd,yd ,V.Container_map)->leaf.order;
        if(AT[pos-1].leaf.is_wall == true)
        {
            AT[x].leaf.on_down= true;
        }
      }

      //check if right sector is a wall
       if(xu > 0 && xu< map_Size && yu > 0 && yu <map_Size)
      {

        int pos = getPosition(xu,yu ,V.Container_map)->leaf.order;
        if(AT[pos-1].leaf.is_wall == true)
        {
            AT[x].leaf.on_up= true;
        }
      }
    }
  }
}



          
        