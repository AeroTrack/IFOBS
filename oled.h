/*-----------------------------------------------------------------------------/
 /	IFOBS - oled.h															   /
 /-----------------------------------------------------------------------------/
 /	Bowie Gian
 /	Created: 2023-06-30
 /	Modified: 2023-07-28
 /
 /	This file contains the function declarations for operating the OLED screen.
 /----------------------------------------------------------------------------*/
#ifndef OLED_H
#define OLED_H

/*--------------------------------------------------------------*/
/* Definitions													*/
/*--------------------------------------------------------------*/

#define OLED_SUCCESS 0
#define OLED_OFF_SCREEN -1

/*--------------------------------------------------------------*/
/* Function Prototypes	    									*/
/*--------------------------------------------------------------*/
// Sets up and clears the OLED
void Oled_setup();

// Turns off all pixels
void Oled_clear();

// Displays the distance at the top
void Oled_displayDistance(int distance);

// Displays ERR at the top
void Oled_displayLidarErr();

// Displays ---m at the top
void Oled_displayDistanceMax();

// Displays the angle on the right
void Oled_displayElevation(double angle);

// Displays the cant at the bottom
void Oled_displayCant(double angle);

// Displays a dot in the center
void Oled_displayCenterDot();

// Display a dot offset down from the center dot
// Returns 0 on success, -1 on failure (off screen)
int Oled_displayCalcDot(int y);

// Clears the calculated dot, if there is one.
// Currently clears all possible locations of this dot.
// Module should keep track of this dot and only clear that byte.
void Oled_clearCalcDot();

#endif
