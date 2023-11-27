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
#include "utils.h"

/*--------------------------------------------------------------*/
/* Definitions													*/
/*--------------------------------------------------------------*/

#define PIN_VSYS_ADC_EN 25
#define PIN_VSYS 29

#define BATT_LOW  2.8
#define BATT_HIGH 4.2

/*--------------------------------------------------------------*/
/* Global Variables				 								*/
/*--------------------------------------------------------------*/

static double voltsBuff[AVERAGING_SIZE] = {0};
static int voltsIndex = 0;
static bool isVoltsInit = false;

static const double threshold = .25;
static int prevReturn = 0;

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
	double volts = (result * 3.3 / (1 << 12)) * 3;
	double averagedVolts = movingAverage(volts, voltsBuff, &voltsIndex, &isVoltsInit);

	double scale = 4.0 / (BATT_HIGH - BATT_LOW);
	double scaledResult = (averagedVolts - BATT_LOW) * scale;

	if (scaledResult > (double)prevReturn + 1.0 + threshold) {
		prevReturn++;
	} else if (scaledResult < (double)prevReturn - threshold) {
		prevReturn--;
	}

	if (prevReturn > 4) {
		prevReturn = 4;
	} else if (prevReturn < 0) {
		prevReturn = 0;
	}

	// printf("Value: %x\n", result);
	printf("Voltage: %f V\n", averagedVolts);
	// printf("Batt: %f\n", scaledResult);
	// printf("Return: %d\n", prevReturn);

	return prevReturn;
}
