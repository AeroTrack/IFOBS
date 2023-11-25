/*---------------------------------------------------------------------------- /
/	IFOBS - oled.c															   /
/ ---------------------------------------------------------------------------- /
/	Bowie Gian
/	Created: 2023-06-30
/	Modified: 2023-07-28
/
/	This file contains the functions that will drive the OLED screen.
/	The SPI setup is modified from the accelerometer example.
/ ----------------------------------------------------------------------------*/

/*--------------------------------------------------------------*/
/* Include Files												*/
/*--------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "oled.h"
#include "lidar.h"

/*--------------------------------------------------------------*/
/* Definitions													*/
/*--------------------------------------------------------------*/

#define DEBUG 1

// 0 = Dot, 1 = Cross
#define DOT_OR_CROSS 1

#define NUM_BRIGHTNESS 8
#define BRIGHTNESS_DISPLAY_LENGTH 5
#define DISABLE_HOLD_LENGTH 10

// Pins
#define PIN_CS		5 // SPI CS
#define PIN_SCK		2 // SPI CLK
#define PIN_MOSI	3 // SPI MOSI
#define PIN_DC		1 // Data/Command pin (Functions need to change)
#define PIN_RST		4 // LOW = reset, keep HIGH

#define BUTTON_UP	6
#define BUTTON_DOWN	7

// Values for PIN_DC
#define OLED_DC_COMD 0 // command
#define OLED_DC_DATA 1 // data

#define DOT_CENTER_COL 0x3C
#define DOT_CENTER_PAGE 0x05
#define DIST_DISP_COL (DOT_CENTER_COL - 0x08)
#define DIST_DISP_PAGE 0x07

#define BATTERY_SYMBOL_LEN 16

/*--------------------------------------------------------------*/
/* Global Variables				 								*/
/*--------------------------------------------------------------*/

static uint8_t curCalcDotCol = DOT_CENTER_COL;
static uint8_t curCalcDotPage = 0x03;

static bool prevButtonUp = false;
static bool prevButtonDown = false;

static int brightnessIndex = 7;
static const uint8_t brightnessSettings[NUM_BRIGHTNESS] = {
	0x00, 0x22, 0x44, 0x66, 0x88, 0xAA, 0xCC, 0xFF
};

// Positive: display and decrement each poll
//  0: clear display and decrement to -1
// -1: do nothing
static int brightnessDisplayCount = -1;

// Positive: buttons held, decrement each poll
//  0: toggle disableStats, decrement to -1
// -1: do nothing
static int bothButtonHoldCount = 0;
static bool disableStats = false;

#if DOT_OR_CROSS == 1
static int prevXOffset = 0;
static int prevYOffset = 0;
#endif

static spi_inst_t *spi;

// Pixel arrays for OLED
// Change to struct with its length for the future ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static uint8_t number[10][3] = {
	{0xFE, 0x82, 0xFE},	// 0
	{0x42, 0xFE, 0x02},	// 1
	{0x9E, 0x92, 0xF2},	// 2
	{0x92, 0x92, 0xFE},	// 3
	{0xF0, 0x10, 0xFE},	// 4
	{0xF2, 0x92, 0x9E},	// 5
	{0xFE, 0x92, 0x9E},	// 6
	{0x80, 0x80, 0xFE},	// 7
	{0xFE, 0x92, 0xFE},	// 8
	{0xF0, 0x90, 0xFE}	// 9
};

static uint8_t letterE[3] = {0xFE, 0x92, 0x92};
static uint8_t letterM[5] = {0x1E, 0x10, 0x0E, 0x10, 0x0E};
static uint8_t letterL[3] = {0xFE, 0x02, 0x02};
static uint8_t letterR[3] = {0xFE, 0xB0, 0xEE};
static uint8_t symbolDeg[3] = {0xE0, 0xA0, 0xE0};
static uint8_t symbolPlus[3] = {0x10, 0x38, 0x10};
static uint8_t symbolNeg[3] = {0x10, 0x10, 0x10};
static uint8_t symbolLock[5] = {0x0E, 0x7E, 0x4A, 0x7E, 0x0E};

static uint8_t symbolBattery[5][BATTERY_SYMBOL_LEN] = {
	{0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, 0x18},
	{0xFF, 0x81, 0xBD, 0xBD, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, 0x18},
	{0xFF, 0x81, 0xBD, 0xBD, 0x81, 0xBD, 0xBD, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, 0x18},
	{0xFF, 0x81, 0xBD, 0xBD, 0x81, 0xBD, 0xBD, 0x81, 0xBD, 0xBD, 0x81, 0x81, 0x81, 0x81, 0xFF, 0x18},
	{0xFF, 0x81, 0xBD, 0xBD, 0x81, 0xBD, 0xBD, 0x81, 0xBD, 0xBD, 0x81, 0xBD, 0xBD, 0x81, 0xFF, 0x18}
};

/*--------------------------------------------------------------*/
/*  Static Function Implemetations								*/
/*--------------------------------------------------------------*/

static void setupSPI()
{
	spi = spi0; // Link spi0 port

	// Initialize SPI port at 1 MHz
	spi_init(spi, 1000 * 1000);

	// Set SPI format
	spi_set_format( spi0,   // SPI instance
					8,      // Number of bits per transfer
					1,      // Polarity (CPOL)
					1,      // Phase (CPHA)
					SPI_MSB_FIRST);

	// Initialize SPI pins
	gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
	gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
}

// Initializes GPIO pins
// CS: HIGH, DC: LOW, RST: HIGH
static void setupGPIO()
{
	gpio_init(PIN_CS);
	gpio_set_dir(PIN_CS, GPIO_OUT);
	gpio_put(PIN_CS, 1);

	gpio_init(PIN_DC);
	gpio_set_dir(PIN_DC, GPIO_OUT);
	gpio_put(PIN_DC, OLED_DC_COMD);

	gpio_init(PIN_RST);
	gpio_set_dir(PIN_RST, GPIO_OUT);
	gpio_put(PIN_RST, 1);

	gpio_init(BUTTON_UP);
	gpio_set_dir(BUTTON_UP, GPIO_IN);
	gpio_pull_up(BUTTON_UP);

	gpio_init(BUTTON_DOWN);
	gpio_set_dir(BUTTON_DOWN, GPIO_IN);
	gpio_pull_up(BUTTON_DOWN);
}

static void setColumnRange(uint8_t start, uint8_t end)
{
	uint8_t data;

	gpio_put(PIN_DC, OLED_DC_COMD);
	gpio_put(PIN_CS, 0);
	{
		// Column Range Command
		data = 0x21;
		spi_write_blocking(spi, &data, 1);

		// Set Start Column
		spi_write_blocking(spi, &start, 1);

		// Set End Column
		spi_write_blocking(spi, &end, 1);
	}
	gpio_put(PIN_CS, 1);
}

static void setPageRange(uint8_t start, uint8_t end)
{
#if DEBUG == 1
	if (start > 0x07)
		printf("DEBUG: setPageRange start value > 7\r\n");

	if (end > 0x07)
		printf("DEBUG: setPageRange end value > 7\r\n");
#endif
	uint8_t data;

	gpio_put(PIN_DC, OLED_DC_COMD);
	gpio_put(PIN_CS, 0);
	{
		// Page Range Command
		data = 0x22;
		spi_write_blocking(spi, &data, 1);

		// Set Start Page
		spi_write_blocking(spi, &start, 1);

		// Set End Page
		spi_write_blocking(spi, &end, 1);
	}
	gpio_put(PIN_CS, 1);
}

// Sends the array of pixels to the OLED
// ASSUMES: PIN_DC = OLED_DC_DATA, PIN_CS = 0
static void display(uint8_t *columnArray, int width)
{
	for (int i = 0; i < width; i++) {
		spi_write_blocking(spi, &columnArray[i], 1);
	}
}

// Clears the calculated dot.
// Module keeps track of this dot and only clears that byte.
static void clearCalcDot()
{
	setColumnRange(curCalcDotCol, curCalcDotCol);
	setPageRange(curCalcDotPage, curCalcDotPage);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		uint8_t blank = 0;
		spi_write_blocking(spi, &blank, 1);
	}
	gpio_put(PIN_CS, 1);
}

// Draws the brightnessIndex + 1 on screen (offset the 0)
static void displayBrightnessSetting()
{
	setColumnRange(DIST_DISP_COL - 0x10, 0x7F);
	setPageRange(0x03, 0x03);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		display(number[brightnessIndex + 1], 3);
	}
	gpio_put(PIN_CS, 1);
}

// Clears the brightnessIndex
static void clearBrightnessSetting()
{
	setColumnRange(DIST_DISP_COL - 0x10, 0x7F);
	setPageRange(0x03, 0x03);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		u_int8_t blank = 0x00;
		for (int i = 0; i < 3; i++) {
			spi_write_blocking(spi, &blank, 1);
		}
	}
	gpio_put(PIN_CS, 1);
}

static void setBrightness(uint8_t brightness) {
	uint8_t data; // Buffer to store output

	gpio_put(PIN_DC, OLED_DC_COMD);
	gpio_put(PIN_CS, 0);
	{
		data = 0x81;
		spi_write_blocking(spi, &data, 1);
		data = brightness;
		spi_write_blocking(spi, &data, 1);
	}
	gpio_put(PIN_CS, 1);

	displayBrightnessSetting();
	brightnessDisplayCount = BRIGHTNESS_DISPLAY_LENGTH;
}

static void brightnessUp() {
	if (brightnessIndex >= NUM_BRIGHTNESS - 1) {
		return;
	}

	brightnessIndex++;
	setBrightness(brightnessSettings[brightnessIndex]);

	printf("Brightness up %d\n", brightnessIndex);
}

static void brightnessDown() {
	if (brightnessIndex <= 0) {
		return;
	}

	brightnessIndex--;
	setBrightness(brightnessSettings[brightnessIndex]);

	printf("Brightness down %d\n", brightnessIndex);
}

/*--------------------------------------------------------------*/
/*  Function Implemetations										*/
/*--------------------------------------------------------------*/

void Oled_setup()
{
	uint8_t data; // Buffer to store output
	
	setupSPI();
	setupGPIO();

	gpio_put(PIN_DC, OLED_DC_COMD);
	gpio_put(PIN_CS, 0);
	{
		// Remap (Flip Horizontally)
		// data = 0xA1;
		// spi_write_blocking(spi, &data, 1);

		// Set Horizonal Addr Mode
		data = 0x20;
		spi_write_blocking(spi, &data, 1);
		data = 0x00;
		spi_write_blocking(spi, &data, 1);
	}
	gpio_put(PIN_CS, 1);

	Oled_clear();

	gpio_put(PIN_DC, OLED_DC_COMD);
	gpio_put(PIN_CS, 0);
	{
		// Set brightness without displaying index
		data = 0x81;
		spi_write_blocking(spi, &data, 1);
		data = brightnessSettings[brightnessIndex];
		spi_write_blocking(spi, &data, 1);
		
		// Turn on display
		data = 0xAF;
		spi_write_blocking(spi, &data, 1);
	}
}

void Oled_brightnessPoll() {
	bool currButtonUp = !gpio_get(BUTTON_UP);
	bool currButtonDown = !gpio_get(BUTTON_DOWN);

	if (currButtonUp != prevButtonUp) {
		if (currButtonUp) {
			brightnessUp();
		}
		prevButtonUp = currButtonUp;
	}

	if (currButtonDown != prevButtonDown) {
		if (currButtonDown) {
			brightnessDown();
		}
		prevButtonDown = currButtonDown;
	}

	if (brightnessDisplayCount > 0) {
		brightnessDisplayCount--;
	} else if (brightnessDisplayCount == 0) {
		clearBrightnessSetting();
		brightnessDisplayCount = -1;
	}

	if (!currButtonUp || !currButtonDown) {
		bothButtonHoldCount = -1;
		return;
	}

	if (bothButtonHoldCount == -1) {
		bothButtonHoldCount = DISABLE_HOLD_LENGTH;
		return;
	}

	if (bothButtonHoldCount == 0) {
		Oled_clear();
		disableStats = !disableStats;
	}

	bothButtonHoldCount--;
}

void Oled_clear()
{
	setColumnRange(0x00, 0x7F);
	setPageRange(0x00, 0x07);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		uint8_t blank = 0;
		for (int i = 0; i < 8*128; i++) {
			spi_write_blocking(spi, &blank, 1);
		}
	}
	gpio_put(PIN_CS, 1);
}

void Oled_displayDistance(int distance_cm)
{
	if (disableStats)
		return;

	setColumnRange(DIST_DISP_COL, 0x7F);
	setPageRange(DIST_DISP_PAGE, DIST_DISP_PAGE);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		uint8_t space = 0;
		if (distance_cm <= LIDAR_DC) {				// If LIDAR disconnected
			display(letterE, 3);
			spi_write_blocking(spi, &space, 1);

			for (int i = 0; i < 2; i++) {
				display(letterR, 3);
				spi_write_blocking(spi, &space, 1);
			}

			// Erase the 'm'
			for (int i = 0; i < 5; i++) {
				spi_write_blocking(spi, &space, 1);
			}
		} else if (distance_cm == LIDAR_MAX_CM) {	// If max distance returned
			for (int i = 2; i >= 0; i--) {
				display(symbolNeg, 3);
				spi_write_blocking(spi, &space, 1);
			}
			display(letterM, 5);
		} else {									// Display distance
			int distance_m = distance_cm / 100;
			int digit[3] = {0};

			digit[0] = distance_m % 10;
			digit[1] = distance_m % 100 / 10;
			digit[2] = distance_m % 1000 / 100;

			for (int i = 2; i >= 0; i--) {
				display(number[digit[i]], 3);
				spi_write_blocking(spi, &space, 1);
			}
			display(letterM, 5);
		}
	}
	gpio_put(PIN_CS, 1);
}

void Oled_displayElevation(double angle)
{
	if (disableStats)
		return;

	bool negative = false;

	if (angle < 0) {
		negative = true;
		angle *= -1;
	}
	
	int angleInt = (int)angle;
	int digit[3] = {0};

	digit[0] = angleInt % 10;
	digit[1] = angleInt % 100 / 10;
	digit[2] = angleInt % 1000 / 100;

	setColumnRange(0x4C, 0x7F);
	setPageRange(0x03, 0x03);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		uint8_t space = 0;

		if (negative) {
			display(symbolNeg, 3);
			spi_write_blocking(spi, &space, 1);
		} else {
			display(symbolPlus, 3);
			spi_write_blocking(spi, &space, 1);
		}
		
		for (int i = 2; i >= 0; i--) {
			display(number[digit[i]], 3);
			spi_write_blocking(spi, &space, 1);
		}
		display(symbolDeg, 3);
	}
	gpio_put(PIN_CS, 1);
}

void Oled_displayCant(double angle)
{
	if (disableStats)
		return;

	bool negative = false;

	if (angle < 0) {
		negative = true;
		angle *= -1;
	}

	int angleInt = (int)angle;
	int digit[3] = {0};

	digit[0] = angleInt % 10;
	digit[1] = angleInt % 100 / 10;
	digit[2] = angleInt % 1000 / 100;

	setColumnRange(0x36, 0x7F);
	setPageRange(0x01, 0x01);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		uint8_t space = 0;

		if (negative) {
			display(letterL, 3);
			spi_write_blocking(spi, &space, 1);
		} else {
			display(letterR, 3);
			spi_write_blocking(spi, &space, 1);
		}
		
		for (int i = 2; i >= 0; i--) {
			display(number[digit[i]], 3);
			spi_write_blocking(spi, &space, 1);
		}
		display(symbolDeg, 3);
	}
	gpio_put(PIN_CS, 1);
}

void Oled_displayCenter()
{
#if DOT_OR_CROSS == 0
	setColumnRange(DOT_CENTER_COL, DOT_CENTER_COL);
	setPageRange(DOT_CENTER_PAGE, DOT_CENTER_PAGE);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		uint8_t dot = 0x01;
		spi_write_blocking(spi, &dot, 1);
	}
	gpio_put(PIN_CS, 1);
#elif DOT_OR_CROSS == 1
	setColumnRange(DOT_CENTER_COL - 6, DOT_CENTER_COL + 6);
	setPageRange(DOT_CENTER_PAGE, DOT_CENTER_PAGE);
	
	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		uint8_t sides = 0x01;
		uint8_t center = 0x78;
		uint8_t blank = 0x00;

		for (int i = 0; i < 13; i++) {
			if (i <= 3 || i >= 9) {
				spi_write_blocking(spi, &sides, 1);
			} else if (i == 6) {
				spi_write_blocking(spi, &center, 1);
			} else {
				spi_write_blocking(spi, &blank, 1);
			}
		}
	}
	gpio_put(PIN_CS, 1);

	setColumnRange(DOT_CENTER_COL, DOT_CENTER_COL);
	setPageRange(DOT_CENTER_PAGE - 1, DOT_CENTER_PAGE - 1);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		uint8_t bottom = 0x3C;
		spi_write_blocking(spi, &bottom, 1);
	}
	gpio_put(PIN_CS, 1);
#endif
}

int Oled_displayCalcDot(int xOffset, int yOffset)
{
#if DOT_OR_CROSS == 0
	// If current dot is on the center dot byte, redraw the center dot to clear
	if (curCalcDotCol == DOT_CENTER_COL && curCalcDotPage == DOT_CENTER_PAGE) {
		Oled_displayCenter();
	} else {
		clearCalcDot();
	}
#elif DOT_OR_CROSS == 1
	clearCalcDot();
#endif
	uint8_t pixel = 0;

	if (xOffset < -16 || xOffset >= 16) {
		// Out of range
		curCalcDotCol = DOT_CENTER_COL;
		curCalcDotPage = DOT_CENTER_PAGE;
#if DOT_OR_CROSS == 1
		Oled_displayCenter();
#endif
		return OLED_OFF_SCREEN;
	}

	if (yOffset < -24 || yOffset >= 16) {
		// Out of range
		curCalcDotCol = DOT_CENTER_COL;
		curCalcDotPage = DOT_CENTER_PAGE;
#if DOT_OR_CROSS == 1
		Oled_displayCenter();
#endif
		return OLED_OFF_SCREEN;
	}

	curCalcDotCol = DOT_CENTER_COL + xOffset;

	curCalcDotPage = (yOffset + DOT_CENTER_PAGE * 8) / 8;
	pixel = 0x01 << ((yOffset + DOT_CENTER_PAGE * 8) % 8);
#if DOT_OR_CROSS == 0
	// If pixel is on the center dot byte, add the center dot too
	if (curCalcDotCol == DOT_CENTER_COL && curCalcDotPage == DOT_CENTER_PAGE) {
		pixel |= 0x01;
	}

	setColumnRange(curCalcDotCol, curCalcDotCol);
	setPageRange(curCalcDotPage, curCalcDotPage);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		spi_write_blocking(spi, &pixel, 1);
	}
	gpio_put(PIN_CS, 1);
#elif DOT_OR_CROSS == 1
	// If inside of crosshair
	if (abs(xOffset) <= 9 && abs(yOffset) <= 9) {
		uint8_t blank = 0;

		// Draw part of crosshair
		setColumnRange(DOT_CENTER_COL - 6, DOT_CENTER_COL + 6);
		setPageRange(DOT_CENTER_PAGE, DOT_CENTER_PAGE);

		gpio_put(PIN_DC, OLED_DC_DATA);
		gpio_put(PIN_CS, 0);
		{
			uint8_t sides = 0x01;
			uint8_t center = 0x78;
			uint8_t blank = 0x00;

			for (int i = 0; i < 13; i++) {
				if (i <= 3 && xOffset >= 0) {
					spi_write_blocking(spi, &sides, 1);
				} else if (i >= 9 && xOffset <= 0) {
					spi_write_blocking(spi, &sides, 1);
				} else if (i == 6 && yOffset <= 0) {
					spi_write_blocking(spi, &center, 1);
				} else {
					spi_write_blocking(spi, &blank, 1);
				}
			}
		}
		gpio_put(PIN_CS, 1);

		if (yOffset >= 0) {
			setColumnRange(DOT_CENTER_COL, DOT_CENTER_COL);
			setPageRange(DOT_CENTER_PAGE - 1, DOT_CENTER_PAGE - 1);

			gpio_put(PIN_DC, OLED_DC_DATA);
			gpio_put(PIN_CS, 0);
			{
				uint8_t bottom = 0x3C;
				spi_write_blocking(spi, &bottom, 1);
			}
			gpio_put(PIN_CS, 1);
		}
	} else {
		Oled_displayCenter();
	}

	// If pixel is on the center dot byte, add the center dot too
	if (xOffset == 0 && yOffset == 0) {
		pixel |= 0x78;
	}

	setColumnRange(curCalcDotCol, curCalcDotCol);
	setPageRange(curCalcDotPage, curCalcDotPage);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		spi_write_blocking(spi, &pixel, 1);
	}
	gpio_put(PIN_CS, 1);

	return OLED_SUCCESS;
#endif
	return OLED_SUCCESS;
}

void Oled_displayLock()
{
	setColumnRange(DIST_DISP_COL - 6, DIST_DISP_COL);
	setPageRange(DIST_DISP_PAGE, DIST_DISP_PAGE);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		display(symbolLock, 5);
	}
	gpio_put(PIN_CS, 1);
}

void Oled_clearLock()
{
	setColumnRange(DIST_DISP_COL - 6, DIST_DISP_COL);
	setPageRange(DIST_DISP_PAGE, DIST_DISP_PAGE);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		u_int8_t blank = 0x00;
		for (int i = 0; i < 5; i++) {
			spi_write_blocking(spi, &blank, 1);
		}
	}
	gpio_put(PIN_CS, 1);
}

void Oled_displayCalcDotErr()
{
	setColumnRange(0x20, 0x20);
	setPageRange(0x02, 0x02);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		u_int8_t error = 0xE4;
		spi_write_blocking(spi, &error, 1);
	}
	gpio_put(PIN_CS, 1);
}

void Oled_clearCalcDotErr()
{
	setColumnRange(0x20, 0x20);
	setPageRange(0x02, 0x02);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		u_int8_t blank = 0x00;
		spi_write_blocking(spi, &blank, 1);
	}
	gpio_put(PIN_CS, 1);
}

void Oled_displayBattery(int fill)
{
	setColumnRange(0x50, 0x7F);
	setPageRange(0x01, 0x01);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		display(symbolBattery[fill], BATTERY_SYMBOL_LEN);
	}
	gpio_put(PIN_CS, 1);
}
