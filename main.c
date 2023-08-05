/*-----------------------------------------------------------------------------/
 /	IFOBS - main.c															   /
 /-----------------------------------------------------------------------------/
 /	Bowie Gian
 /	Created: 2023-06-30
 /	Modified: 2023-07-26
 /
 /	This file contains the main function for the IFOBS.
 /----------------------------------------------------------------------------*/

/*--------------------------------------------------------------*/
/* Include Files												*/
/*--------------------------------------------------------------*/

#include <stdio.h>
#include "pico/stdio.h"
#include "pico/time.h"
#include "tusb.h"
#include "accelerometer.h"
#include "ballistics.h"
#include "lidar.h"
#include "oled.h"

/*--------------------------------------------------------------*/
/* Definitions													*/
/*--------------------------------------------------------------*/

#define SERIAL_MONITOR_WAIT 0

/*--------------------------------------------------------------*/
/* Main Function												*/
/*--------------------------------------------------------------*/

int main()
{
	// Initialize serial port
	stdio_init_all();

	// Time to start monitoring serial port
#if SERIAL_MONITOR_WAIT == 1
	cdcd_init();
	printf("waiting for usb host");
	while (!tud_cdc_connected()) {
		printf(".");
		sleep_ms(500);
	}
	printf("\nusb host detected!\n");
#endif

	Oled_setup();
	Accel_setup();
	Lidar_setup();

	Oled_displayCenterDot();

	while (true) {
		Angle angles;

		Accel_poll();
		angles = Accel_getAngle();

		Lidar_poll();
		short distance_cm = Lidar_getDistanceCm();
		// short distance_cm = 17900;
		double distance_m = (double)distance_cm / 100.0;

		int xOffset = 0;
		int yOffset = 0;
		if (distance_cm != LIDAR_DC && distance_cm != LIDAR_MAX_CM) {
			Ballistics_calculatePixelOffset(distance_m, angles.theta,
					angles.alpha, &xOffset, &yOffset);
		}

		printf("%d %d %d\r\n", distance_cm, xOffset, yOffset);
		Oled_displayDistance(distance_cm);
		Oled_displayElevation(angles.theta);
		Oled_displayCant(angles.alpha);
		int statusOled = Oled_displayCalcDot(xOffset, yOffset); // Use to display warning
		sleep_ms(100);
	}
}
