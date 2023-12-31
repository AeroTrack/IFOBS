/*---------------------------------------------------------------------------- /
/	IFOBS - oled.h															   /
/ ---------------------------------------------------------------------------- /
/	Bowie Gian
/	Created: 2023-06-30
/	Modified: 2023-07-28
/
/	This file contains the function declarations for operating the OLED screen.
/ ----------------------------------------------------------------------------*/
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

// Polls the brightness buttons
void Oled_brightnessPoll();

// Turns off all pixels
void Oled_clear();

// Displays the distance in meters at the top
// If distance_cm is -1, displays ERR at the top
// If distance_cm is 18000, displays ---m at the top
void Oled_displayDistance(int distance_cm);

// Displays the angle on the right
void Oled_displayElevation(double angle);

// Displays the cant at the bottom
void Oled_displayCant(double angle);

// Displays a dot in the center
void Oled_displayCenter();

// Display a dot offset down from the center dot
// Returns 0 on success, -1 on failure (off screen)
int Oled_displayCalcDot(int x, int y);

void Oled_displayLock();

void Oled_clearLock();

void Oled_displayCalcDotErr();

void Oled_clearCalcDotErr();

#endif
