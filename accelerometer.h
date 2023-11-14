/*---------------------------------------------------------------------------- /
/	IFOBS - accelerometer.h													   /
/ ---------------------------------------------------------------------------- /
/	Mint Luc
/	Bowie Gian
/	Created: 2023-06-30
/	Modified: 2023-07-28
/
/	This file contains the function declarations
/	for operating the accelerometer.
/ ----------------------------------------------------------------------------*/
#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

/*--------------------------------------------------------------*/
/* Structs														*/
/*--------------------------------------------------------------*/

typedef struct {
	double r;
	double theta;
	double alpha;
} Angle;

/*--------------------------------------------------------------*/
/* Function Prototypes	    									*/
/*--------------------------------------------------------------*/

void Accel_setup();
void Accel_poll();
Angle Accel_getAngle();

#endif
