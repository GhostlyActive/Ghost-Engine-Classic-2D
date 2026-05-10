#include "Controls.h"
#include <Arduino.h>


/* ============================================================================
 *  Controls.cpp - Bounce2 hardware buttons + Serial Monitor key bridge.
 *  See Controls.h for the public API.
 * ============================================================================ */


/* ----- Hardware buttons ------------------------------------------------- */

// 10 ms debounce, default for Bounce2.
Bounce btn_Bounce_F  = Bounce(BTN_F,  10);
Bounce btn_Bounce_B  = Bounce(BTN_B,  10);
Bounce btn_Bounce_L  = Bounce(BTN_L,  10);
Bounce btn_Bounce_R  = Bounce(BTN_R,  10);
Bounce btn_Bounce_A1 = Bounce(BTN_A1, 10);
Bounce btn_Bounce_A2 = Bounce(BTN_A2, 10);


void Btn_setup()
{
    // INPUT_PULLUP enables the AVR's internal pull-up so the button can
    // simply tie the pin to GND. Pressed = LOW, released = HIGH.
    pinMode(BTN_F,  INPUT_PULLUP);
    pinMode(BTN_B,  INPUT_PULLUP);
    pinMode(BTN_L,  INPUT_PULLUP);
    pinMode(BTN_R,  INPUT_PULLUP);
    pinMode(BTN_A1, INPUT_PULLUP);
    pinMode(BTN_A2, INPUT_PULLUP);
}


/* ----- Serial Monitor bridge -------------------------------------------- */
//
// Each received character sets a one-shot flag. The game loop calls
// Serial_poll() once per iteration and consumes flags via Serial_F() etc.
// Each Serial_X() returns true exactly once per received key, then resets.

static volatile bool flag_F  = false;
static volatile bool flag_B  = false;
static volatile bool flag_L  = false;
static volatile bool flag_R  = false;
static volatile bool flag_A1 = false;
static volatile bool flag_A2 = false;


void Serial_help()
{
    Serial.println();
    Serial.println(F("=== Ghost Engine Classic 2D - Serial Controls ==="));
    Serial.println(F("  w = Forward          a = Rotate left"));
    Serial.println(F("  s = Backward         d = Rotate right"));
    Serial.println(F("  e = Action 1         q = Action 2"));
    Serial.println(F("  h = show this help again"));
    Serial.println(F("=================================================="));
    Serial.println();
}


void Serial_poll()
{
    while (Serial.available() > 0)
    {
        int c = Serial.read();
        if (c < 0) break;

        // Accept uppercase too.
        if (c >= 'A' && c <= 'Z') c += ('a' - 'A');

        switch (c)
        {
            case 'w': flag_F  = true; break;
            case 's': flag_B  = true; break;
            case 'a': flag_L  = true; break;
            case 'd': flag_R  = true; break;
            case 'e': flag_A1 = true; break;
            case 'q': flag_A2 = true; break;
            case 'h': Serial_help(); break;
            // CR / LF / unknown: ignore.
            default: break;
        }
    }
}


// Returns true exactly once per flag set, then clears the flag.
static bool consume(volatile bool& flag)
{
    if (flag) { flag = false; return true; }
    return false;
}

bool Serial_F()  { return consume(flag_F);  }
bool Serial_B()  { return consume(flag_B);  }
bool Serial_L()  { return consume(flag_L);  }
bool Serial_R()  { return consume(flag_R);  }
bool Serial_A1() { return consume(flag_A1); }
bool Serial_A2() { return consume(flag_A2); }
