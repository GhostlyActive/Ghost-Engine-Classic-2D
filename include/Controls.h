#ifndef CONTROLS_H
#define CONTROLS_H

#include "Bounce2.h"


/* ============================================================================
 *  Controls.h
 *
 *  Two parallel input paths:
 *
 *    Hardware buttons (Bounce2-debounced) on digital pins:
 *        F=2, B=3, L=4, R=5, A1=6, A2=7
 *      Polling style is up to the caller:
 *          btn_Bounce_X.fell()       -> single edge, "tap"
 *          btn_Bounce_X.read() == 0  -> currently held, "hold"
 *
 *    Serial Monitor keys (115200 baud):
 *        w = Forward         s = Backward
 *        a = Rotate left     d = Rotate right
 *        e = Action 1        q = Action 2
 *        h = print this help again
 *      Each keystroke is a one-shot edge: Serial_F() returns true exactly
 *      once per received 'w'.
 *
 *  Typical use in a game loop:
 *      Serial_poll();                 // drain bytes -> set flags
 *      btn_Bounce_F.update();
 *      if (btn_Bounce_F.read() == 0) ...  // continuous walk
 *      if (Serial_F())              ...  // discrete keystroke step
 * ============================================================================ */


/* ----- Hardware button pins --------------------------------------------- */

#define BTN_F   2
#define BTN_B   3
#define BTN_L   4
#define BTN_R   5
#define BTN_A1  6
#define BTN_A2  7


/* ----- Bounce2 instances (defined in Controls.cpp) ---------------------- */

extern Bounce btn_Bounce_F;
extern Bounce btn_Bounce_B;
extern Bounce btn_Bounce_L;
extern Bounce btn_Bounce_R;
extern Bounce btn_Bounce_A1;
extern Bounce btn_Bounce_A2;


/* ----- Setup + Serial bridge -------------------------------------------- */

void Btn_setup();          // pinMode + pullups for all six buttons

void Serial_help();        // print the key map to the Serial Monitor
void Serial_poll();        // drain Serial buffer -> set one-shot flags

bool Serial_F();           // 'w' pressed since last call
bool Serial_B();           // 's'
bool Serial_L();           // 'a'
bool Serial_R();           // 'd'
bool Serial_A1();          // 'e'
bool Serial_A2();          // 'q'


#endif
