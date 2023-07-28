/*-----------------------------------------------------------------------------/
 /	IFOBS - lidar.h															   /
 /-----------------------------------------------------------------------------/
 /	Mint Luc
 /  Bowie Gian
 /	Created: 2023-06-30
 /	Modified: 2023-07-28
 /
 /	This file contains the function declarations for operating the LIDAR.
 /----------------------------------------------------------------------------*/
#ifndef LIDAR_H
#define LIDAR_H

/*--------------------------------------------------------------*/
/* Definitions													*/
/*--------------------------------------------------------------*/

#define LIDAR_DC -1
#define LIDAR_MAX_CM 18000

/*--------------------------------------------------------------*/
/* Function Prototypes	    									*/
/*--------------------------------------------------------------*/

void Lidar_setup();
void Lidar_poll();

// Returns the most recent polled distance in cm
// Returns -1 if LIDAR is disconnected
short Lidar_getDistanceCm();

#endif
