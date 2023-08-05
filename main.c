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
/* Global Variables				 								*/
/*--------------------------------------------------------------*/

short distance_cm = 0;
double distance_m = 0;

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

	Oled_displayCenter();

	while (true) {
		Angle angles;

		Accel_poll();
		angles = Accel_getAngle();

		Lidar_buttonPoll();

		if (Lidar_isLocked()) {
			printf("Distance Locked\r\n");
			Oled_displayLock();
		} else {
			printf("Distance Unlocked\r\n");
			Oled_clearLock();

			Lidar_distancePoll();
			distance_cm = Lidar_getDistanceCm();
			// distance_cm = 17900;
			distance_m = (double)distance_cm / 100.0;
		}

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

		int statusOled = Oled_displayCalcDot(xOffset, yOffset);

		if (statusOled == OLED_OFF_SCREEN) {
			Oled_displayCalcDotErr();
		} else if (statusOled == OLED_SUCCESS) {
			Oled_clearCalcDotErr();
		}
		
		sleep_ms(50);
	}
}
