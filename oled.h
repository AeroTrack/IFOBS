/*-----------------------------------------------------------------------------/
 /	IFOBS - oled.h															   /
 /-----------------------------------------------------------------------------/
 /	Bowie Gian
 /	Created: 2023-06-30
 /	Modified: 2023-06-30
 /
 /	This file contains the function declarations for operating the OLED screen.
 /----------------------------------------------------------------------------*/
#ifndef OLED_H
#define OLED_H

/*--------------------------------------------------------------*/
/* Function Prototypes	    									*/
/*--------------------------------------------------------------*/
void Oled_setup();
void Oled_clear();
void Oled_displayDistance();
void Oled_displayElevation(double angle);
void Oled_displayCant(double angle);
void Oled_displayCenterDot();

// Display a dot offset down from the center dot
void Oled_displayCalcDot(int y);
void OLED_DisplayTest();

#endif
