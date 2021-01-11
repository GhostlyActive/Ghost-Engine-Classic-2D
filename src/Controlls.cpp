#include "Controlls.h"

/*

We are using the Bounce2 for debouncing of the buttons
https://github.com/thomasfredericks/Bounce2

We instantiate 4 Objects for forward, backward, left rotation and right rotation and 2 for action

*/


// Instantiate Bounce objects
    Bounce btn_Bounce_F = Bounce(BTN_F, 10);
    Bounce btn_Bounce_B = Bounce(BTN_B, 10);
    Bounce btn_Bounce_L = Bounce(BTN_L, 10);
    Bounce btn_Bounce_R = Bounce(BTN_R, 10);
    Bounce btn_Bounce_A1 = Bounce(BTN_A1, 10);
    Bounce btn_Bounce_A2 = Bounce(BTN_A2, 10);


void Btn_setup(){

    //declare Pins as Inputs
    pinMode(BTN_F, INPUT_PULLUP);
 	pinMode(BTN_B, INPUT);
    pinMode(BTN_L, INPUT);
    pinMode(BTN_R, INPUT);
    pinMode(BTN_A1, INPUT);
    pinMode(BTN_A2, INPUT);
  
    // set Pins to High
  	digitalWrite(BTN_F, HIGH);  
  	digitalWrite(BTN_B, HIGH);
    digitalWrite(BTN_L, HIGH);
    digitalWrite(BTN_R, HIGH);
  	digitalWrite(BTN_A1, HIGH);
  	digitalWrite(BTN_A2, HIGH);
}