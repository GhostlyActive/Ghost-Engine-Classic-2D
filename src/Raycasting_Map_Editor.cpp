#include "Raycasting_Map_Editor.h"

// 2d Map on OLED SCREEN which can be modified. 
// outer borders are always 1. Player can change everything inside of this walls.
// Color change by clicking on action button 

void buildMapEditor::Load_Map_Editor(Adafruit_SSD1351 tft){

  tft.fillScreen(BLACK);

  //border of the map gets the value 1
  for(int i = 0; i < size_x; i++){

    worldMap[0][i] = 1;
    worldMap[size_x-1][i] = 1;
    worldMap[i][0] = 1;
    worldMap[i][size_y-1] = 1;

        // setup Pinmodes for controll
      Btn_setup();
  }

  for(;;)
  {
    //target cross is moveable around matrix. and screening alle elements with chosen color

    tft.drawFastVLine(position_x*5.5,0,SCREEN_HEIGHT,WHITE);
    tft.drawFastHLine(0,position_y*5.5,SCREEN_WIDTH,WHITE);

    for(int x = 0; x<size_x;x++)
    {
      for(int y = 0; y<size_y;y++)
      {

        if(worldMap[y][x] == 0)
        {

          tft.fillRect(x*5.5,y*5.5,3,3,BLACK);

        }

        if(worldMap[y][x] == 1)
        {

          tft.fillRect(x*5.5,y*5.5,3,3,WHITE);

        }

        if(worldMap[y][x] == 2)
        {

          tft.fillRect(x*5.5,y*5.5,3,3,BLUE);

        }

        if(worldMap[y][x] == 3)
        {

          tft.fillRect(x*5.5,y*5.5,3,3,GREEN);

        }

        if(worldMap[y][x] == 4)
        {

          tft.fillRect(x*5.5,y*5.5,3,3,RED);

        }

      }
    }

       /********************************************Controlls***********************************************
        ****************************************************************************************************/
        // action_2 (start)
        // up
        // down
        // left
        // right
        // action_1 (select)


        //start game and engine
        // button action_2
        btn_Bounce_A2.update();
		    if(btn_Bounce_A2.fell())
        {

          if(worldMap[position_y][position_x] == 0)
          {

            //Player position and abilities
            Player p;
            p.posX = position_y;
            p.posY = position_x;
            Load_Ray_Engine(tft, p);
          }
        }

        // go up
        btn_Bounce_F.update();
		    if(btn_Bounce_F.fell())  
        {  

          tft.fillScreen(BLACK);

          if(position_y > 1) 
          {
            position_y--;
          }

        }

        // go down
        btn_Bounce_B.update();
		    if(btn_Bounce_B.fell())
        {  

          tft.fillScreen(BLACK);

          if(position_y < size_y-2)
          {
            position_y++;
          }
        }

        // go left
        btn_Bounce_L.update();
		    if(btn_Bounce_L.fell())
        {  

          tft.fillScreen(BLACK);
        
          if(position_x > 1) 
          {

           position_x--;

          }
        }


        // go right
        btn_Bounce_R.update();
		    if(btn_Bounce_R.fell())
        {  

          tft.fillScreen(BLACK);
        
          if(position_x < size_x-2){

          position_x++;

          }
        }

      // Button Action_1 
      btn_Bounce_A1.update();
		  if(btn_Bounce_A1.fell())
      {  
        // change color when pushed
        switch (worldMap[position_y][position_x])
        {

          case 0:
          worldMap[position_y][position_x] = 2;
            break;

          case 2:
          worldMap[position_y][position_x]= 3;
          break;

          case 3:
          worldMap[position_y][position_x] = 4;
          break;

          case 4:
          worldMap[position_y][position_x] = 0;
          break;

          default: 
          worldMap[position_y][position_x] = 0;
          break;
        }
      }
  }
}


