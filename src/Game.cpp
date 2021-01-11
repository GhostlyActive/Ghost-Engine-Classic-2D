#include "Game.h"
#include "Controlls.h"



void Game_start(Adafruit_SSD1351 tft)
{
    BSPEditor editor;
    View V = Save_BSP_Engine();
    editor.Load_BSP_Editor(tft, V.test, V);
}



void Game_loop(Adafruit_SSD1351 tft, BSP_Player P, View V)
{
	// setup pinmodes for controll
	Btn_setup();
	// draw status bar at the bottom of the display
    tft.drawBitmap(0, screenHeight-screenStatistics, stats, 128, 20, YELLOW);	
	tft.setCursor(25, screenHeight-15);
    tft.setTextSize(2);
	// show init Health
	tft.println(P.health);
	// draw compass
	compass(tft, P, V);

	for(;;)
	{
		// rendering engine start. update playerAngle
		FieldOfView2Angle(V);

		// draw from the nearest wall to the farthest
		PrintCloseToFar(V.test,getPosition(P.player_px, P.player_py, V.Container_map)->leaf.order, tft, P, V);

		for(int x = 0; x<screenWidth;x++)
		{
			if(xBuffer[x] == false)
			{
				if(yBuffer[x]!= 0)
				{

				tft.drawFastVLine(x,((screenHeight-screenStatistics)-1)/2-yBuffer[x]/2,yBuffer[x]+1,BLACK);
				
				}

				yBuffer[x] = 0;
				colorBuffer[x] = BLACK;
			}
		}

		//reset clipping buffer
		memset(xBuffer, 0, screenWidth*sizeof(bool));


		// draw Hand at the end, because it must be in the foreground
    	tft.drawBitmap(screenWidth/2, screenHeight-60, Hand, 40, 40, WHITE);


		/********************************************Controlls***********************************************
		****************************************************************************************************/
		// action
		// forward
		// backward
		// rotate right
		// rotate left


		// Action button to place or delete a wall in front of the player. when there is an enemy -> neutralize and increase health 
		// Button action is clicked. first update status of button. When it was clicked -> code below is executed
		btn_Bounce_A1.update();
		if(btn_Bounce_A1.fell())
		{
			float p_x = sinf(V.playerAngle-PI/2);
			float p_y = cosf(V.playerAngle-PI/2); 

			// get position of 50 units away sector from player
			int position = getPosition(P.player_px-p_x * 50 , P.player_py-p_y * 50,V.Container_map)->leaf.order;

			if( V.test[position-1].leaf.is_wall == false)
			{
				if(V.test[position-1].leaf.is_enemy == true)
				{
					P.health = P.health+25;
					V.test[position-1].leaf.is_enemy = false;
					tft.fillRect(25,SCREEN_HEIGHT-20, 35, 20,BLACK);

					tft.setCursor(25, screenHeight-15);
					tft.setTextSize(2);

					tft.println(P.health);

					V.test[position-1].leaf.is_enemy = false;
				}

				V.test[position-1].leaf.is_wall = true;
				V.test[position-1].leaf.color = BLUE;
				game_Near_Walls(V.test, V, 1);		
			}

			else
			{
				V.test[position-1].leaf.is_wall = false; 
				V.test[position-1].leaf.color = BLACK;
				game_Near_Walls(V.test, V, 0);

			}
		}


		// move forward
		// button forward is clicked. first update status of button. When it was clicked -> code below is executed
		btn_Bounce_F.update();
		if(btn_Bounce_F.fell())
		{  
			
			float p_x = sinf(V.playerAngle-PI/2);	// single vectors from Player Angle
			float p_y = cosf(V.playerAngle-PI/2); 	// single vectors from Player Angle

			int position = getPosition(P.player_px-p_x * P.playerMove , P.player_py-p_y * P.playerMove,V.Container_map)->leaf.order;


			if(V.test[position-1].leaf.is_wall == false && (position >= 0  && position <= V.rooms))
			{
				compass(tft, P, V);	// refresh compass

				if(V.test[position-1].leaf.order == P.target_sector)
				{
					
					win(tft);			// show win text and goto menu view

				}
				// when next position is an enemy sector -> health decrease. when <=0 -> the end. 
				if(V.test[position-1].leaf.is_enemy == true)
				{
					P.health = P.health-25;

					if(P.health <=0)
					{
						lose(tft);
					}

					V.test[position-1].leaf.is_enemy = false;
					tft.fillRect(25,SCREEN_HEIGHT-20, 35, 20,BLACK);

					tft.setCursor(25, screenHeight-15);
					tft.setTextSize(2);

					// print actual health
					tft.println(P.health);
				}
				
	
				P.player_px = P.player_px-p_x * P.playerMove;  // direction multiplied by playerMove

				P.player_py = P.player_py-p_y * P.playerMove;  // direction multiplied by playerMove
			}
		}


		//move Backward
		// Button backward is clicked. first update status of button. When it was clicked -> code below is executed
		btn_Bounce_B.update();
		if(btn_Bounce_B.fell())
		{  
			float p_x = sinf(V.playerAngle-PI/2);
			float p_y = cosf(V.playerAngle-PI/2); 

			int position = getPosition(P.player_px+p_x * P.playerMove , P.player_py+p_y*P.playerMove,V.Container_map)->leaf.order;

			if(V.test[position-1].leaf.is_wall == false && (position >= 0  && position <= V.rooms))
			{
				compass(tft, P, V);		// refresh compass

				if(V.test[position-1].leaf.order == P.target_sector)
				{

					win(tft);

				}
				// when next position is an enemy sector -> health decrease. when <=0 -> the end. 
				if(V.test[position-1].leaf.is_enemy == true)
				{
					P.health = P.health-25;

					if(P.health <=0)
					{
						lose(tft);
					}

					V.test[position-1].leaf.is_enemy = false;
					tft.fillRect(25,SCREEN_HEIGHT-20, 35, 20,BLACK);

					tft.setCursor(25, screenHeight-15);
					tft.setTextSize(2);

					// print actual health
					tft.println(P.health);
				}

				P.player_px = P.player_px+p_x * P.playerMove;  	// direction multiplied by playerMove

				P.player_py = P.player_py+p_y*P.playerMove;  	// direction multiplied by playerMove
			}
		}


		//rotate right
		// Button rotate_right is clicked. first update status of button. When it was clicked -> code below is executed
		btn_Bounce_R.update();
		if(btn_Bounce_R.fell())
		{

			V.playerAngle = V.playerAngle-P.playerTurn;

			compass(tft, P, V);		// refresh compass
		}


		// roate left
		// Button rotate_left is clicked. first update status of button. When it was clicked -> code below is executed
		btn_Bounce_L.update();
		if(btn_Bounce_L.fell())
		{

			V.playerAngle = V.playerAngle+P.playerTurn;

			compass(tft, P, V);		// refresh compass								
		} 
	}
}



/********************************************TEXT***********************************************
************************************************************************************************/

// win text
void win(Adafruit_SSD1351 menu)
{
	buildMenu build;

	menu.fillScreen(BLACK);
    menu.drawRect(0,0,128,128,YELLOW);

    menu.setCursor(10, (screenHeight/2)-10);
    menu.setTextSize(2);

    menu.println("   WIN");
    
	delay(1000);
	
	build.ShowMenu(menu);
}

// lose text
void lose(Adafruit_SSD1351 menu)
{
	buildMenu build;

	menu.fillScreen(BLACK);
    menu.drawRect(0,0,128,128,YELLOW);

    menu.setCursor(10, (screenHeight/2)-10);
    menu.setTextSize(2);

    menu.println("  lose");
    
	delay(1000);
	
	build.ShowMenu(menu);
}


/********************************************GAMEPLAY ELEMENTS***********************************
************************************************************************************************/

//show compass. Following the compass needle towards the north leads to victory
// playerAngle - (angle from player to target) -> when difference is 0, player is looking towards target, then needle points north.     ||     When != not on roead

void compass(Adafruit_SSD1351 menu, BSP_Player P, View V)
{
	//delete old compass needle on this rectangle place
	menu.fillRect(100, 110, 120, 130, BLACK);

	// save playerAngle
	float backup_playerA = V.playerAngle*180/PI;
	// playerAngle always between 0 - 360
	backup_playerA = backup_playerA + ceil(-backup_playerA / 360 ) *360;


	float delta_x = ( P.target_x - P.player_px);
	float delta_y = ( P.target_y - P.player_py);

	// angle between positv x-axis and vector(player-target)
	float angle = atan(delta_y/delta_x)*180/PI;

	// angle always between 0 - 360
	angle = angle + ceil(-angle / 360) * 360;

	angle = 180-angle;

	// drawing compass needle 
	menu.drawLine(110, 120, 110 + 10*sin((backup_playerA*PI/180)-angle*PI/180), 120 + 10*cos((backup_playerA*PI/180)-angle*PI/180), YELLOW);
}




// 1 -> is getting a wall and 0 -> is getting void
void game_Near_Walls(struct Tree* AT, View V, int mode)
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
      if(xl > 0 && xl< map_Size && yl > 0 && yl <map_Size )
      {
        int pos = getPosition(xl,yl ,V.Container_map)->leaf.order;
        if(AT[pos-1].leaf.is_wall == true)
        {
			if(mode == 1)
			{
            	AT[x].leaf.on_left = true;
			}
			if(mode == 0)
			{
				AT[pos-1].leaf.on_right = false;
				AT[x].leaf.on_left = false;
			}
        }
      }

      //check if right sector is a wall
      if(xr > 0 && xr< map_Size && yr > 0 && yr <map_Size)
      {

      int pos = getPosition(xr,yr ,V.Container_map)->leaf.order;
      if(AT[pos-1].leaf.is_wall == true)
        {
			if(mode == 1)
			{
            	AT[x].leaf.on_right = true;
			}
			if(mode == 0)
			{
				AT[pos-1].leaf.on_left = false;
				AT[x].leaf.on_right = false;

			}
        }
      }

      //check if down sector is a wall
      if(xd > 0 && xd< map_Size && yd > 0 && yd <map_Size)
      {

        int pos = getPosition(xd,yd ,V.Container_map)->leaf.order;
        if(AT[pos-1].leaf.is_wall == true)
        {
			if(mode == 1)
			{
            	AT[x].leaf.on_down = true;
			}
			if(mode == 0)
			{
				AT[pos-1].leaf.on_up = false;
				AT[x].leaf.on_down = false;
			}
        }
      }

      //check if right sector is a wall
       if(xu > 0 && xu< map_Size && yu > 0 && yu <map_Size)
      {

        int pos = getPosition(xu,yu ,V.Container_map)->leaf.order;
        if(AT[pos-1].leaf.is_wall == true)
        {
			if(mode == 1)
			{
            	AT[x].leaf.on_up = true;
			}
			if(mode == 0)
			{
				AT[pos-1].leaf.on_down = false;
				AT[x].leaf.on_up = false;
			}
        }
      }
    }
  }
}



/********************************************TEXTURE********************************************
************************************************************************************************/


// Textures are made with GIMP. Than convertet into this byte-array form
// converter: http://javl.github.io/image2cpp/


// Hand texture (Bitmap)

    unsigned char Hand  [] = {
   	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x80, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x01, 0xc1, 
	0xc6, 0x00, 0x00, 0x03, 0xe1, 0xc6, 0x00, 0x00, 0x03, 0xe1, 0xc7, 0x00, 0x00, 0x01, 0xe1, 0xc7, 
	0x00, 0x00, 0x00, 0xf1, 0xc7, 0x00, 0x00, 0x00, 0x71, 0xc7, 0x00, 0x00, 0x00, 0x78, 0xc7, 0x00, 
	0x00, 0x00, 0x78, 0xc7, 0x00, 0x00, 0x00, 0x7c, 0xc7, 0x06, 0x00, 0x00, 0x3c, 0xe7, 0x06, 0x00, 
	0x00, 0x1f, 0xff, 0x0e, 0x00, 0x00, 0x1f, 0xfe, 0x1e, 0x00, 0x00, 0x1f, 0xfe, 0x3c, 0x00, 0x00, 
	0x1f, 0xff, 0x3c, 0x00, 0x00, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0xf8, 0x00, 0x00, 0x1f, 
	0xff, 0xf8, 0x00, 0x00, 0x1f, 0xff, 0xf8, 0x00, 0x00, 0x1f, 0xff, 0xf8, 0x00, 0x07, 0x1f, 0xff, 
	0xf8, 0x00, 0x07, 0xff, 0xff, 0xf8, 0x00, 0x07, 0xff, 0xff, 0xfc, 0x00, 0x03, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0x80, 0x00, 0x1f, 0xff, 0xff, 0xe0, 0x00, 0x0f, 0xff, 0xff, 0xf0, 
	0x00, 0x01, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x00, 
	0x00, 0x0f, 0xff, 0xff, 0x00, 0x00, 0x07, 0xff, 0xff, 0x00, 0x00, 0x03, 0xff, 0xff, 0x00, 0x00, 
	0x01, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff
};


// status bar texture

unsigned char stats  [] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0xc7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x03, 0xef, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x07, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x0f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x0f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x0f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x0f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x0f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x0f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x07, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x03, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};