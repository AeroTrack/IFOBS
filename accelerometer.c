/*---------------------------------------------------------------------------- /
/	IFOBS - accelerometer.c													   /
/ ---------------------------------------------------------------------------- /
/	Mint Luc
/	Bowie Gian
/	Created: 2023-06-30
/	Modified: 2023-11-19
/
/	This file contains the functions that will drive the accelerometer.
/	The SPI setup is modified from the accelerometer example.
/ ----------------------------------------------------------------------------*/

/*--------------------------------------------------------------*/
/* Include Files												*/
/*--------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "accelerometer.h"

/*--------------------------------------------------------------*/
/* Definitions													*/
/*--------------------------------------------------------------*/

#define DEBUG 0

// Pins
#define CS_PIN 13
#define SCK_PIN 14
#define MOSI_PIN 15
#define MISO_PIN 12

// Registers
#define REG_DEVID 0x00
#define REG_POWER_CTL 0x2D
#define REG_DATAX0 0x32

#define DEVID 0xE5

#define AVERAGING_SIZE 5

/*--------------------------------------------------------------*/
/* Global Variables				 								*/
/*--------------------------------------------------------------*/

// Other constants
static const float SENSITIVITY_2G = 1.0 / 256;	// (g/LSB)
static const float EARTH_GRAVITY = 9.80665;		// Earth's gravity in [m/s^2]

// Ports
static spi_inst_t *spi = spi1;

// Angle output
static Angle angles;

static double thetaBuff[AVERAGING_SIZE] = {0};
static int thetaIndex = 0;
static bool isThetaInit = false;

static double alphaBuff[AVERAGING_SIZE] = {0};
static int alphaIndex = 0;
static bool isAlphaInit = false;

/*--------------------------------------------------------------*/
/*  Static Function Implemetations								*/
/*--------------------------------------------------------------*/

// Write 1 byte to the specified register
static void reg_write(	spi_inst_t *spi, const uint cs,
						const uint8_t reg, const uint8_t data)
{
	uint8_t msg[2];
				
	// Construct message (set ~W bit low, MB bit low)
	msg[0] = 0x00 | reg;
	msg[1] = data;

	// Write to register
	gpio_put(cs, 0);
	spi_write_blocking(spi, msg, 2);
	gpio_put(cs, 1);
}

// Read byte(s) from specified register. If nbytes > 1, read from consecutive
// registers.
static int reg_read(spi_inst_t *spi, const uint cs, const uint8_t reg,
					uint8_t *buf, const uint8_t nbytes)
{
	int num_bytes_read = 0;
	uint8_t mb = 0;

	// Determine if multiple byte (MB) bit should be set
	if (nbytes < 1) {
		return -1;
	} else if (nbytes == 1) {
		mb = 0;
	} else {
		mb = 1;
	}

	// Construct message (set ~W bit high)
	uint8_t msg = 0x80 | (mb << 6) | reg;

	// Read from register
	gpio_put(cs, 0);
	spi_write_blocking(spi, &msg, 1);
	num_bytes_read = spi_read_blocking(spi, 0, buf, nbytes);
	gpio_put(cs, 1);

	return num_bytes_read;
}

// Inputs the next value and returns the new average
static double movingAverage(double value, double buffer[], int *pIndex, bool *pIsInit) {
	if (!*pIsInit) {
		*pIsInit = true;

		for (int i = 0; i < AVERAGING_SIZE; i++) {
			buffer[i] = value;
		}
		return value;
	}

	if (*pIndex < AVERAGING_SIZE - 1) {
		*pIndex += 1;
	} else {
		*pIndex = 0;
	}

	buffer[*pIndex] = value;

	double sum = 0.0;

#if DEBUG == 1
	printf("[ ");
#endif
	for (int i = 0; i < AVERAGING_SIZE; i++) {
		sum += buffer[i];
#if DEBUG == 1
		printf("%lf ", buffer[i]);
#endif
	}

	double average = sum / (double)AVERAGING_SIZE;
#if DEBUG == 1
	printf("] Avg: %lf\n", average);
#endif
	return average;
}

static double movAvgTheta(double value) {
	return movingAverage(value, thetaBuff, &thetaIndex, &isThetaInit);
}

static double movAvgAlpha(double value) {
	return movingAverage(value, alphaBuff, &alphaIndex, &isAlphaInit);
}

// Limitation: Averaging doesn't work when optic is upside down
// due to fluctuations near 180 and -180
static Angle cal_Angle(double x, double y, double z) {
	double sum_r, r, theta, alpha;
	Angle result_angle;

	sum_r =  x*x + y*y + z*z;
	result_angle.r = sqrt(sum_r);

	theta = atan2(y, sqrt(x * x + z * z)) * 180/M_PI;
	result_angle.theta = movAvgTheta(theta);

	alpha = atan2(x, -z) * 180/M_PI;
	result_angle.alpha = movAvgAlpha(alpha);

	return result_angle; 
}

/*--------------------------------------------------------------*/
/*  Function Implemetations										*/
/*--------------------------------------------------------------*/

void Accel_setup()
{
	// Buffer to store raw reads
	uint8_t data[6];

	// Initialize chosen serial port
	stdio_init_all();

	// Initialize CS pin high
	gpio_init(CS_PIN);
	gpio_set_dir(CS_PIN, GPIO_OUT);
	gpio_put(CS_PIN, 1);

	// Initialize SPI port at 1 MHz
	spi_init(spi, 1000 * 1000);

	// Set SPI format
	spi_set_format(	spi1,	// SPI instance
					8,		// Number of bits per transfer
					1,		// Polarity (CPOL)
					1,		// Phase (CPHA)
					SPI_MSB_FIRST);

	// Initialize SPI pins
	gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);
	gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);
	gpio_set_function(MISO_PIN, GPIO_FUNC_SPI);

	// Workaround: perform throw-away read to make SCK idle high
	reg_read(spi, CS_PIN, REG_DEVID, data, 1);

	// Read device ID to make sure that we can communicate with the ADXL343
	reg_read(spi, CS_PIN, REG_DEVID, data, 1);
	if (data[0] != DEVID) {
		printf("ERROR: Could not communicate with ADXL343\r\n");
		while (true);
	}

	// Read Power Control register
	reg_read(spi, CS_PIN, REG_POWER_CTL, data, 1);
	printf("0x%X\r\n", data[0]);

	// Tell ADXL343 to start taking measurements by setting Measure bit to high
	data[0] |= (1 << 3);
	reg_write(spi, CS_PIN, REG_POWER_CTL, data[0]);

	// Test: read Power Control register back to make sure Measure bit was set
	reg_read(spi, CS_PIN, REG_POWER_CTL, data, 1);
	printf("0x%X\r\n", data[0]);

	// Wait before taking measurements
	sleep_ms(2000);
}

void Accel_poll()
{
	// Buffer to store raw reads
	uint8_t data[6];

	// Read X, Y, and Z values from registers (16 bits each)
	reg_read(spi, CS_PIN, REG_DATAX0, data, 6);

	// Convert 2 bytes (little-endian) into 16-bit integer (signed)
	int16_t acc_x = (int16_t)((data[1] << 8) | data[0]);
	int16_t acc_y = (int16_t)((data[3] << 8) | data[2]);
	int16_t acc_z = (int16_t)((data[5] << 8) | data[4]);

	// Convert measurements to [m/s^2]
	float acc_x_f = acc_x * SENSITIVITY_2G * EARTH_GRAVITY;
	float acc_y_f = acc_y * SENSITIVITY_2G * EARTH_GRAVITY;
	float acc_z_f = acc_z * SENSITIVITY_2G * EARTH_GRAVITY;

	// Print results
	//printf("X: %.2f | Y: %.2f | Z: %.2f\r\n", acc_x_f, acc_y_f, acc_z_f);
	angles = cal_Angle(acc_x_f, acc_y_f, acc_z_f);

	//printf("r: %.2f | theta: %.2f | alpha: %.2f\r\n", angles.r, angles.theta, angles.alpha);
}

Angle Accel_getAngle()
{
	return angles;
}
