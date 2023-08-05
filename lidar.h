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

// Sets up the LIDAR UART and GPIO pins
void Lidar_setup();

// Toggles the LIDAR lock state
void Lidar_buttonPoll();

// Polls the LIDAR, the distance is returned from Lidar_getDistanceCm()
void Lidar_distancePoll();

// Returns the most recent polled distance in cm
// Returns -1 if LIDAR is disconnected
short Lidar_getDistanceCm();

// Returns if the LIDAR is locked,
bool Lidar_isLocked();

#endif
