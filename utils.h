/*---------------------------------------------------------------------------- /
/	IFOBS - utils.h															   /
/ ---------------------------------------------------------------------------- /
/	Bowie Gian
/	Created: 2023-11-27
/	Modified: 2023-11-27
/
/	This file contains utility function declarations used by multiple modules.
/ ----------------------------------------------------------------------------*/
#ifndef UTILS_H
#define UTILS_H

/*--------------------------------------------------------------*/
/* Definitions													*/
/*--------------------------------------------------------------*/

#define AVERAGING_SIZE 5

/*--------------------------------------------------------------*/
/* Function Prototypes	    									*/
/*--------------------------------------------------------------*/

// Inputs the next value and returns the new average
double movingAverage(double value, double buffer[], int *pIndex, bool *pIsInit);

#endif
