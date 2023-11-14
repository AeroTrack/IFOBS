/*---------------------------------------------------------------------------- /
/	IFOBS - ballistics.h													   /
/ ---------------------------------------------------------------------------- /
/	Mint Luc
/	Bowie Gian
/	Created: 2023-06-30
/	Modified: 2023-07-28
/
/	This file contains the function declarations for the ballistics
/	calculations.
/ ----------------------------------------------------------------------------*/
#ifndef BALLISTICS_H
#define BALLISTICS_H

// Calculates the bullet drop and returns the offsets to xOffset and yOffset
void Ballistics_calculatePixelOffset(double distance_m, double elev_deg,
		double cant_deg, int *xOffset, int *yOffset);

#endif
