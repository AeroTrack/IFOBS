/*---------------------------------------------------------------------------- /
/	IFOBS - ballistics.h													   /
/ ---------------------------------------------------------------------------- /
/	Mint Luc
/	Bowie Gian
/	Hong Shi
/	Created: 2023-06-30
/	Modified: 2023-11-17
/
/	This file contains the function declarations for the ballistics calculations.
/
/
/	coordinate system
/
/	z yaw	^
/			|
/			|
/ 			|_______> y cant
/			/
/	 	   /
/		  /  x elevation
/		 v
/ ----------------------------------------------------------------------------*/

#ifndef BALLISTICS_H
#define BALLISTICS_H

// Description: Calculates the bullet drop and returns the offsets as a pixel value to xOffset and zOffset 
//				screen pixel offset is a positive or negative int with respect to a (0,0) center screen
// Input :
//			distance_m:		distance from optic to target measured by lazer range finder
//							this Value should be locked via the range finder lock button
//
//			elev_deg:		rifle elevation with: 0deg  -> aimed flat down range
//												 90deg -> aimed to the sky
//												-90deg -> aimed at the g 
//
//			cant_deg:		rifle cant with:  0deg -> aimed flat down range
//											 90deg -> rifle rotated 90deg to its right
//											-90deg -> rifle rotated 90deg to its left
//							                        
//			xOffset:		+1 -> 1 pixel to the right
//							-1 -> 1 pixel to the left
//
//			zOffset:		+1 -> 1 pixel upwards
//							+1 -> 1 pixel downwards
//
// Output :
//			None

void Ballistics_calculatePixelOffset(double distance_m, double elev_deg, double cant_deg, int *xOffset, int *zOffset);

#endif
