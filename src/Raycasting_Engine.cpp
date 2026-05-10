#include "Raycasting_Engine.h"


/*
 * Based on Lode's Computer Graphics Tutorial (vector/camera version):
 *   https://lodev.org/cgtutor/raycasting.html
 *
 * Uses the Adafruit SSD1351 OLED driver.
 *
 * Adds a Y-buffer and color buffer to speed up SPI drawing on the screen.
 */

void Load_Ray_Engine(Adafruit_SSD1351 tft, Player p)
{

  // Clear screen
  tft.fillScreen(BLACK);

  // Y buffer for saving line heights in every column
  int yBuffer[screenWidth] = {0};

  // Color buffer for all columns
  uint16_t  colorBuffer[screenWidth] = {0};

  // Set up pin modes for controls
	Btn_setup();

  for(;;)
  {
    // Serial monitor input (parallel to hardware buttons)
    Serial_poll();

    for(int x = 0; x < screenWidth; x++)
    {
      double cameraX = 2 * x / (double)(screenWidth) - 1; // x-coordinate in camera space
      double rayDirX = p.dirX + p.planeX * cameraX;
      double rayDirY = p.dirY + p.planeY * cameraX;

      // Which box of the map we're in
      int mapX = int(p.posX);
      int mapY = int(p.posY);


      // Length of ray from current position to next x or y-side
        double sideDistX;
        double sideDistY;

        // Length of ray from one x or y-side to next x or y-side
        // scaleFactor = 1/rayDirX;  **-> scaledX = rayDirX * (1/rayDirX) = 1  **->  scaledY = rayDirY * (1/rayDirX) = rayDirY/rayDirX
        // length = sqrt((x * x) + (y * y))  **-> length = sqrt((1 * 1 ) + (rayDirY / rayDirX) * (rayDirY / rayDirX))
        //
        // deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX))
        // deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY))
        double deltaDistX = abs(1 / rayDirX);
        double deltaDistY = abs(1 / rayDirY);
        double perpWallDist;

    
        //what direction to step in x or y-direction (either +1 or -1)
        int stepX;
        int stepY;

        int hit = 0; // Was there a wall hit?
        int side;    // Was a NS or a EW wall hit?
        // Calculate step and initial sideDist
        if(rayDirX < 0)
        {
          stepX = -1;
          sideDistX = (p.posX - mapX) * deltaDistX;
        }
        else
        {
          stepX = 1;
          sideDistX = (mapX + 1.0 - p.posX) * deltaDistX;
        }
        if(rayDirY < 0)
        {
          stepY = -1;
          sideDistY = (p.posY - mapY) * deltaDistY;
        }
        else
        {
          stepY = 1;
          sideDistY = (mapY + 1.0 - p.posY) * deltaDistY;
        }

        //perform DDA
        while (hit == 0)
        {
          // Jump to next map square, either in x or y direction
          if(sideDistX < sideDistY)
          {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
          }
          else
          {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
          }
          // Check if ray has hit a wall
          if(worldMap[mapX][mapY] > 0)
          {
          hit = 1;
          }
        }


        // Calculate distance projected on camera direction (Euclidean distance will give fisheye effect!)
        if(side == 0) perpWallDist = (mapX - p.posX + (1 - stepX) / 2) / rayDirX;
        else          perpWallDist = (mapY - p.posY + (1 - stepY) / 2) / rayDirY;
        

        int lineHeight;
        // Calculate height of line to draw on screen
        if(perpWallDist >= 1 )
        {
        lineHeight = abs((int)(2*screenHeight / perpWallDist));
        }

      else
        {
        lineHeight = screenHeight;
        }

      uint16_t Wall_col;

      switch(worldMap[mapX][mapY])
      {
      case 1: Wall_col = WHITE; break;
      case 2: Wall_col = BLUE; break;
      case 3: Wall_col = GREEN; break;
      case 4: Wall_col = RED; break;
      default: Wall_col = WHITE; break;
      }


      // With the help of YBuffer we can draw or delete wall height only when needed. YBuffer saves drawn heights of the walls and colorBuffer saves drawn color.
      if(yBuffer[x] != lineHeight)
      {

        // If same color
        if(colorBuffer[x] == Wall_col)
        {
        int differenz = yBuffer[x]-lineHeight;
        int one = (128- yBuffer[x])/2;
        int two = (128-lineHeight)/2;

        // New line is getting smaller. Delete just the unwanted lines above and below the wall.
        if(differenz > 0)
        {
          tft.drawFastVLine(x, one, two-one, BLACK);                  //above 
          tft.drawFastVLine(x, screenWidth-two, two-one, BLACK);      //below
        }


        // New line is getting bigger
        else
        {
          tft.drawFastVLine(x, two, one-two, Wall_col);
          int begin_y = screenWidth-two-(one-two)-1;
          tft.drawFastVLine(x, begin_y, one-two, Wall_col);
        }

        // If another color
        }
        else
        {
          int start_2 = (128-lineHeight)/2;
          tft.drawFastVLine(x,0,128,BLACK);
          tft.drawFastVLine(x,start_2,lineHeight, Wall_col);
        }
      }

      // If yBuffer and lineHeight have the same value, but color has changed
      else
      {
        if(colorBuffer[x] != Wall_col)
        {
          int start_3 = (128-lineHeight)/2;
          tft.drawFastVLine(x,start_3,lineHeight,Wall_col);
        }
      }


      // Save actual lineHeight and Wall_col to buffers
      yBuffer[x] = lineHeight;
      colorBuffer[x] = Wall_col;
    
    }


		// ----- Controls -----
		// forward / backward / rotate right / rotate left


    //Rotations with rotation matrix
    //collision detection included with    if( ...  == 0) -> no wall -> move available



		// move forward
		// button forward is clicked. first update status of button. When it was clicked -> code below is executed
		btn_Bounce_F.update();
		if(btn_Bounce_F.fell() || Serial_F())
    {
      //do not forget that worldmap_array is [up/down][left/right] so [x][y]
      if(worldMap[int(p.posX + p.dirX * p.moveSpeed)][int(p.posY)] == 0)
      {
          p.posX += p.dirX * p.moveSpeed;
      }
      
      if(worldMap[int(p.posX)][int(p.posY + p.dirY * p.moveSpeed)] == 0)
      {
          p.posY += p.dirY * p.moveSpeed;
      }
    }


		//move Backward
		// Button backward is clicked. first update status of button. When it was clicked -> code below is executed
		btn_Bounce_B.update();
		if(btn_Bounce_B.fell() || Serial_B())
    {
      if(worldMap[int(p.posX - p.dirX * p.moveSpeed)][int(p.posY)] == 0)
      {
          p.posX -= p.dirX * p.moveSpeed;
      }

      if(worldMap[int(p.posX)][int(p.posY - p.dirY * p.moveSpeed)] == 0)
      {
          p.posY -= p.dirY * p.moveSpeed;
      }
    }

    // Rotate right
    // Button rotate_right is clicked. First update status of button. When it was clicked -> code below is executed
		btn_Bounce_R.update();
		if(btn_Bounce_R.fell() || Serial_R())
    {
        double oldXDir = p.dirX;
        p.dirX = p.dirX*cos(-p.rotSpeed) - p.dirY*sin(-p.rotSpeed);
        p.dirY = oldXDir*sin(-p.rotSpeed) + p.dirY*cos(-p.rotSpeed);

        double oldXPlane = p.planeX;
        p.planeX = p.planeX*cos(-p.rotSpeed) - p.planeY*sin(-p.rotSpeed);
        p.planeY = oldXPlane*sin(-p.rotSpeed) + p.planeY*cos(-p.rotSpeed);
    }


    // Rotate left
    // Button rotate_left is clicked. First update status of button. When it was clicked -> code below is executed
		btn_Bounce_L.update();
		if(btn_Bounce_L.fell() || Serial_L())
    {
        double oldXDir = p.dirX;
        p.dirX = p.dirX*cos(p.rotSpeed) - p.dirY*sin(p.rotSpeed);
        p.dirY = oldXDir*sin(p.rotSpeed) + p.dirY*cos(p.rotSpeed);

        double oldXPlane = p.planeX;
        p.planeX = p.planeX*cos(p.rotSpeed) - p.planeY*sin(p.rotSpeed);
        p.planeY = oldXPlane*sin(p.rotSpeed) + p.planeY*cos(p.rotSpeed);   
    }
      //exit(0); //exit the loop
 }
}
