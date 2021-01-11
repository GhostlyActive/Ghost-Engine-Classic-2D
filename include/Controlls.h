#ifndef CONTROLLS_h
#define CONTROLLS_h

#include "Bounce2.h"

#define BTN_F 2   	// Button forward
#define BTN_B 3   	// Button Backward
#define BTN_L 4   	// Button Rotation_left
#define BTN_R 5  	// Button Rotation_right
#define BTN_A1 6   	// Button Action_1
#define BTN_A2 7   	// Button Action_2

extern Bounce btn_Bounce_F;
extern Bounce btn_Bounce_B;
extern Bounce btn_Bounce_L;
extern Bounce btn_Bounce_R;
extern Bounce btn_Bounce_A1;
extern Bounce btn_Bounce_A2;

void Btn_setup();





#endif