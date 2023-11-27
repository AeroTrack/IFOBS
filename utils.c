/*---------------------------------------------------------------------------- /
/	IFOBS - utils.c															   /
/ ---------------------------------------------------------------------------- /
/	Bowie Gian
/	Created: 2023-11-27
/	Modified: 2023-11-27
/
/	This file contains utility functions used by multiple modules.
/ ----------------------------------------------------------------------------*/

/*--------------------------------------------------------------*/
/* Include Files												*/
/*--------------------------------------------------------------*/

#include <stdio.h>
#include <stdbool.h>
#include "utils.h"

/*--------------------------------------------------------------*/
/* Definitions													*/
/*--------------------------------------------------------------*/

#define DEBUG 0

/*--------------------------------------------------------------*/
/*  Function Implemetations										*/
/*--------------------------------------------------------------*/

// Inputs the next value and returns the new average
double movingAverage(double value, double buffer[], int *pIndex, bool *pIsInit) {
	if (!*pIsInit) {
		*pIsInit = true;

		for (int i = 0; i < AVERAGING_SIZE; i++) {
			buffer[i] = value;
		}
		return value;
	}

	if (*pIndex < AVERAGING_SIZE - 1) {
		*pIndex += 1;
	} else {
		*pIndex = 0;
	}

	buffer[*pIndex] = value;

	double sum = 0.0;

#if DEBUG == 1
	printf("[ ");
#endif
	for (int i = 0; i < AVERAGING_SIZE; i++) {
		sum += buffer[i];
#if DEBUG == 1
		printf("%lf ", buffer[i]);
#endif
	}

	double average = sum / (double)AVERAGING_SIZE;
#if DEBUG == 1
	printf("] Avg: %lf\n", average);
#endif
	return average;
}
