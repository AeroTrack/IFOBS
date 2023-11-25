/*---------------------------------------------------------------------------- /
/	IFOBS - battery.c														   /
/ ---------------------------------------------------------------------------- /
/	Bowie Gian
/	Created: 2023-11-24
/	Modified: 2023-11-24
/
/	This file contains the function implementations that will monitor the battery.
/ ----------------------------------------------------------------------------*/

/*--------------------------------------------------------------*/
/* Include Files												*/
/*--------------------------------------------------------------*/

#include <stdio.h>
#include "hardware/adc.h"
#include "battery.h"

/*--------------------------------------------------------------*/
/* Definitions													*/
/*--------------------------------------------------------------*/

#define PIN_VSYS_ADC_EN 25
#define PIN_VSYS 29

#define BATT_LOW  2.8
#define BATT_HIGH 4.2

/*--------------------------------------------------------------*/
/*  Function Implemetations										*/
/*--------------------------------------------------------------*/

void Battery_setup()
{
	// This pin enables GPIO29 ADC pin to read VSYS
	gpio_init(PIN_VSYS_ADC_EN);
	gpio_set_dir(PIN_VSYS_ADC_EN, GPIO_OUT);
	gpio_put(PIN_VSYS_ADC_EN, 1);

	// Init ADC & pin
	adc_init();
	adc_gpio_init(PIN_VSYS);
	adc_select_input(3);
}

int Battery_get()
{
	uint16_t result = adc_read();
	float resultF = (result * 3.3f / (1 << 12)) * 3;

	float scale = 5.0f / (BATT_HIGH - BATT_LOW);
	float scaledResult = (resultF - BATT_LOW) * scale;

	printf("Value: %x\n", result);
	printf("Voltage: %f V\n", resultF);
	printf("Batt: %f\n", scaledResult);

	if (scaledResult > 4.0f) {
		return 4;
	} else if (scaledResult > 3.0f) {
		return 3;
	} else if (scaledResult > 2.0f) {
		return 2;
	} else if (scaledResult > 1.0f) {
		return 1;
	} else {
		return 0;
	}
}
