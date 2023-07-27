/*-----------------------------------------------------------------------------/
 /	IFOBS - oled.c															   /
 /-----------------------------------------------------------------------------/
 /	Bowie Gian
 /	Created: 2023-06-30
 /	Modified: 2023-06-30
 /
 /	This file contains the functions that will drive the OLED screen.
 /	The SPI setup is modified from the accelerometer example.
 /----------------------------------------------------------------------------*/

/*--------------------------------------------------------------*/
/* Include Files												*/
/*--------------------------------------------------------------*/
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "oled.h"

/*--------------------------------------------------------------*/
/* Definitions													*/
/*--------------------------------------------------------------*/
#define DEBUG 1
// Pins
#define PIN_CS		5 // SPI CS
#define PIN_SCK		2 // SPI CLK
#define PIN_MOSI	3 // SPI MOSI
#define PIN_DC		1 // Data/Command pin
#define PIN_RST		4 // LOW = reset, keep HIGH

// Values for PIN_DC
#define OLED_DC_COMD 0 // command
#define OLED_DC_DATA 1 // data

/*--------------------------------------------------------------*/
/* Global Variables				 								*/
/*--------------------------------------------------------------*/
static spi_inst_t *spi;
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

static uint8_t letterM[5] = {0x1E, 0x10, 0x0E, 0x10, 0x0E};
static uint8_t letterL[3] = {0xFE, 0x02, 0x02};
static uint8_t letterR[3] = {0xFE, 0xB0, 0xEE};
static uint8_t symbolDeg[3] = {0xE0, 0xA0, 0xE0};
static uint8_t symbolPlus[3] = {0x10, 0x38, 0x10};
static uint8_t symbolNeg[3] = {0x10, 0x10, 0x10};

/*--------------------------------------------------------------*/
/*  Function Implemetations										*/
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

// Initialize pins
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
}

void Oled_setup()
{
	uint8_t data; // Buffer to store output
	
	setupSPI();
	setupGPIO();

	gpio_put(PIN_CS, 0);
	{
		// Turn on display
		data = 0xAF;
		spi_write_blocking(spi, &data, 1);

		// Remap (Flip Horizontally)
		data = 0xA1;
		spi_write_blocking(spi, &data, 1);

		// Set Horizonal Addr Mode
		data = 0x20;
		spi_write_blocking(spi, &data, 1);
		data = 0x00;
		spi_write_blocking(spi, &data, 1);
	}
	gpio_put(PIN_CS, 1);
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

void Oled_clear()
{
	setColumnRange(0x00, 0x7F);
	setPageRange(0x00, 0x07);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		uint8_t data = 0;
		for (int i = 0; i < 8*128; i++) {
			spi_write_blocking(spi, &data, 1);
		}
	}
	gpio_put(PIN_CS, 1);
}

static void display(uint8_t *columnArray, int width)
{
	for (int i = 0; i < width; i++) {
		spi_write_blocking(spi, &columnArray[i], 1);
	}
}

void Oled_displayDistance(int distance)
{
	setColumnRange(0x38, 0x7F);
	setPageRange(0x07, 0x07);

	int digit[3] = {0};

	digit[0] = distance % 10;
	digit[1] = distance % 100 / 10;
	digit[2] = distance % 1000 / 100;

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		uint8_t space = 0;
		for (int i = 2; i >= 0; i--) {
			display(number[digit[i]], 3);
			spi_write_blocking(spi, &space, 1);
		}
		display(letterM, 5);
	}
	gpio_put(PIN_CS, 1);
}

void Oled_displayElevation(double angle)
{
	bool negative = false;

	setColumnRange(0x60, 0x7F);
	setPageRange(0x04, 0x04);

	if (angle < 0) {
		negative = true;
		angle *= -1;
	}
	
	int angleInt = (int)angle;
	int digit[3] = {0};

	digit[0] = angleInt % 10;
	digit[1] = angleInt % 100 / 10;
	digit[2] = angleInt % 1000 / 100;

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
	bool negative = false;

	setColumnRange(0x36, 0x7F);
	setPageRange(0x00, 0x00);

	if (angle < 0) {
		negative = true;
		angle *= -1;
	}
	
	int angleInt = (int)angle;
	int digit[3] = {0};

	digit[0] = angleInt % 10;
	digit[1] = angleInt % 100 / 10;
	digit[2] = angleInt % 1000 / 100;

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

void Oled_displayCenterDot()
{
	setColumnRange(0x40, 0x40);
	setPageRange(0x04, 0x04);

	gpio_put(PIN_DC, OLED_DC_DATA);
	uint8_t dot = 0x01;
	gpio_put(PIN_CS, 0);
	{
		display(&dot, 1);
	}
	gpio_put(PIN_CS, 1);
}

void Oled_displayCalcDot(int y)
{
	if (y <= 0 || y > 24) {
		// Out of range
		return;
	}

	int page = 3 - (y - 1) / 8;
	uint8_t pixel = 0x80 >> ((y - 1) % 8);

	setColumnRange(0x40, 0x40);
	setPageRange(page, page);

	gpio_put(PIN_DC, OLED_DC_DATA);
	gpio_put(PIN_CS, 0);
	{
		display(&pixel, 1);
	}
	gpio_put(PIN_CS, 1);
}
