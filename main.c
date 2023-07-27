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
	Lidar_Setup();

	while (true) {
		Angle angles;

		Accel_poll();
		angles = Accel_getAngle();

		Lidar_Poll();
		unsigned short distance_cm = Lidar_GetDistanceCm();
		double distance_m = (double)distance_cm / 100.0;

		double yDrop = ballisticsTest(distance_m);
		int pixelOffset = calculatePixelOffset(distance_m, yDrop);

		Oled_clear();
		Oled_displayDistance((int)distance_m);
		Oled_displayElevation(-angles.theta);
		Oled_displayCant(-angles.alpha);
		Oled_displayCenterDot();
		Oled_displayCalcDot(pixelOffset);
		sleep_ms(100);
	}
}
