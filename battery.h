/*---------------------------------------------------------------------------- /
/	IFOBS - battery.h														   /
/ ---------------------------------------------------------------------------- /
/	Bowie Gian
/	Created: 2023-11-24
/	Modified: 2023-11-24
/
/	This file contains the function declarations for monitoring the battery.
/ ----------------------------------------------------------------------------*/
#ifndef BATTERY_H
#define BATTERY_H

void Battery_setup();

// Returns 0-4 depending on the battery percentage
int Battery_get();

#endif
