#include "Menu.h"

//#irst thing which is seen on the OLED Display
//ShowMenu -> start , settings , raycast 
//ShowCircle is the graphical position of the circle. 
//position 1 is start, position 2 is settings and position 3 is raycast

buildMenu::buildMenu(){

    setPosition(0);

}


buildMenu::~buildMenu(){

}


void buildMenu:: ShowMenu(Adafruit_SSD1351 menu)
{
    menu.fillScreen(BLACK);

    // setup pinmodes for controll
	Btn_setup();

    //play, settings, raycast on OLED Display
    for( ; ; )
    {
        menu.setTextColor(YELLOW);
        menu.setCursor(5 + 20, radius*2);
        menu.setTextSize(2);
        menu.println("PLAY");
        menu.setTextSize(2);

        menu.setCursor(5 + 20, (screenHeight/2)-radius);
        menu.setTextSize(2);
        menu.println("SETTINGS");
        menu.setTextSize(2);

        menu.setCursor(5 + 20, screenHeight-(radius*4));
        menu.setTextSize(2);
        menu.println("RAYCAST");
        menu.setTextSize(2);


        /********************************************Controlls***********************************************
        ****************************************************************************************************/
        // up
        // down
        // action_1 (select)

        // go up
        btn_Bounce_F.update();
	    if(btn_Bounce_F.fell())  
        {  
            int inkrement = getPosition();
            inkrement--;
            setPosition(inkrement);

            menu.fillScreen(BLACK);

            if(getPosition() <= 1)
            {
                setPosition(1);
                ShowCircle(menu, 5, radius*3);
            }

            if(getPosition() == 2 )
            {
                ShowCircle(menu, 5, screenWidth/2);

            }
        }


        // go down
        btn_Bounce_B.update();
		if(btn_Bounce_B.fell())
        {  
            int inkrement = getPosition();
            inkrement++;
            setPosition(inkrement);


            menu.fillScreen(BLACK);

            if(getPosition() <= 1)
            {

                ShowCircle(menu, 5, radius*3);

            }

            if(getPosition() == 2){

                ShowCircle(menu, 5, screenWidth/2);

            }

            if(getPosition() >= 3){

                ShowCircle(menu, 5, screenWidth-(radius*3));
                setPosition(3);

            }
        }


        // Button Action_1 
        btn_Bounce_A1.update();
		if(btn_Bounce_A1.fell())
        {

            if(getPosition() == 3)
            {
                //Class Object for MapEditor
                buildMapEditor buildMap;

                //load into MapEditor
                buildMap.Load_Map_Editor(menu);
                
            }

            // when clicked it will move from iteration 4 to 6 to 8  and from 8 to 4
            if(getPosition() == 2)
            {

                if(iteration == 4)
                {

                    iteration = 6;
                }

                else if(iteration == 6)
                {

                    iteration = 8;
                }

                else if(iteration == 8)
                {

                    iteration = 4;
                }
                
                menu.fillScreen(BLACK);
                menu.drawRect(0,0,128,128,RED);

                menu.setTextColor(YELLOW);
                menu.setCursor(10, radius*2);
                menu.setTextSize(2);
                menu.println("ITERATION");
                menu.setTextSize(2);

                menu.setCursor(screenWidth/2-10, (screenHeight/2)-radius);
                menu.setTextSize(2);
                menu.println(iteration);
                menu.setTextSize(4);
                

                delay(500);
                menu.fillScreen(BLACK);
                ShowCircle(menu, 5, screenWidth/2);
            }

            if(getPosition() == 1)
            {

                Game_start(menu);
              
            }
        }
    }
}



void buildMenu::ShowCircle(Adafruit_SSD1351 menu , int x,  int y){

    menu.setTextColor(YELLOW);
    menu.fillCircle(x, y, radius, YELLOW);
}

int buildMenu::getPosition(){

    return position;
}


void buildMenu::setPosition(int x){

    position = x;
}

