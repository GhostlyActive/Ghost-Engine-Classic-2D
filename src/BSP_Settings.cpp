#include "BSP_Settings.h"

//clipping Buffer
bool xBuffer[screenWidth] = {0};

// Y Buffer for saving lineHeights in every columns
int yBuffer[screenWidth] = {0};

// Color Buffer for all columns
uint16_t colorBuffer[screenWidth] = {0};

// how many sector do we want. 2^(iteration) sectors. 
int iteration = 6;


