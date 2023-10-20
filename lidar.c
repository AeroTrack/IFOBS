/*-----------------------------------------------------------------------------/
 /	IFOBS - liar.c															   /
 /-----------------------------------------------------------------------------/
 /	Mint Luc
 /	Bowie Gian
 /	Created: 2023-06-30
 /	Modified: 2023-07-28
 /
 /	This file contains the functions that will setup and poll the LIDAR.
 /----------------------------------------------------------------------------*/

/*--------------------------------------------------------------*/
/* Include Files												*/
/*--------------------------------------------------------------*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/uart.h"
#include "lidar.h"

/*--------------------------------------------------------------*/
/* Definitions													*/
/*--------------------------------------------------------------*/

// which port we want to use uart0 or uart1
// #define UART_ID0 uart0
#define BAUD_RATE 115200
#define UART_ID1 uart1

// We are using pins 0 and 1 for uart0 and pins 11 and 12 for uart1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.

#define UART1_TX_PIN 8 // pin-11
#define UART1_RX_PIN 9 // pin-12

#define PIN_BUTTON 11
#define PIN_5V_REG 16

/*--------------------------------------------------------------*/
/* Global Variables				 								*/
/*--------------------------------------------------------------*/

const uint LED_PIN = 25; // also set LED from gpio.h file
static bool ret;
static bool isLocked = false;
static bool prevButtonState = false;

//************************Structure and Union for handling LiDAR Data***********

//Dist_L Dist_H Strength_L Strength_H Temp_L Temp_H Checksum
typedef struct {
	unsigned short Header;
	short Dist;
	unsigned short Strength;
} structLidar;

union unionLidar {
	unsigned char Byte[9];
	structLidar lidar;
};

unsigned char lidarCounter = 0;
union unionLidar Lidar;

//************************Structure and Union for handling LiDAR Data***********

/*--------------------------------------------------------------*/
/*  Static Function Implemetations								*/
/*--------------------------------------------------------------*/

// Setup distance lock button and 5V step up
static void setupGPIO()
{
	gpio_init(PIN_BUTTON);
	gpio_set_dir(PIN_BUTTON, GPIO_IN);
	gpio_pull_up(PIN_BUTTON);

	gpio_init(PIN_5V_REG);
	gpio_set_dir(PIN_5V_REG, GPIO_OUT);
	gpio_put(PIN_5V_REG, 1);
}

// Function to read serial data
static int isLidar(uart_inst_t * uart, union unionLidar * lidar)
{
	int loop;
	int checksum;
	unsigned char serialChar;

	while (uart_is_readable(uart)) {
		if (lidarCounter > 8) {
			lidarCounter=0;
			return 0; // something wrong
		}

		serialChar = uart_getc(uart); // Read a single character to UART.
		lidar->Byte[lidarCounter] = serialChar;

		switch (lidarCounter++)
		{
		case 0:
		case 1:
			if (serialChar !=0x59)
				lidarCounter=0;
			break;
		case 8: // checksum
			checksum = 0;
			lidarCounter = 0;

			for (loop=0;loop<8;loop++)
				checksum+= lidar->Byte[loop];

			if ((checksum &0xff) == serialChar) {
				//printf("checksum ok\n");
				lidar->lidar.Dist = lidar->Byte[2] | lidar->Byte[3] << 8;
				lidar->lidar.Strength = lidar->Byte[4] | lidar->Byte[5] << 8;
				return 1;
			}
			//printf("bad checksum %02x != %02x\n",checksum & 0xff, serialChar);
		}
	}
	return 0;
}

/*--------------------------------------------------------------*/
/*  Function Implemetations										*/
/*--------------------------------------------------------------*/

void Lidar_setup()
{
	setupGPIO();

	//******************************************************************
	// add some binary info
	// we need to add pico/binary_info.h for this.
	// bi_decl(bi_program_description("This is a program to read from UART!"));
	// bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));
	// add binary info for uart0 and uart1
	
	bi_decl(bi_1pin_with_name(UART1_TX_PIN, "pin-5 for uart1 TX"));
	bi_decl(bi_1pin_with_name(UART1_RX_PIN, "pin-6 for uart1 RX"));
	//******************************************************************

	// gpio_init(LED_PIN); // initialize pin-25
	// gpio_set_dir(LED_PIN, GPIO_OUT); // set pin-25 in output mode

	// Set up our UARTs with the required speed.
	// uart_init(UART_ID0, BAUD_RATE);
	uart_init(UART_ID1, BAUD_RATE);

	// Set the TX and RX pins by using the function
	// Look at the datasheet for more information on function select

	gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);

	// In a default system, printf will also output via the default UART
	sleep_ms(200);
	ret = uart_is_enabled(uart1); // pass UART_ID1 or uart1 both are okay
		if(ret == true) {
			printf("UART-1 is enabled\n");
		}
	printf("Ready to read data\n");
}

void Lidar_buttonPoll()
{
	bool currButtonState = !gpio_get(PIN_BUTTON);
	if (currButtonState == prevButtonState) {
		return;
	} else if (currButtonState) {
		isLocked = !isLocked;
		prevButtonState = currButtonState;
	} else {
		prevButtonState = currButtonState;
	}
}

void Lidar_distancePoll()
{
	// gpio_put(LED_PIN, 0);
	// sleep_ms(200);
	// gpio_put(LED_PIN, 1);
	if (isLidar(UART_ID1,&Lidar)) {
		printf("Dist:%u \n", Lidar.lidar.Dist);
	} else {
		printf("LIDAR disconnected\r\n");
		Lidar.lidar.Dist = LIDAR_DC;
	}
}

short Lidar_getDistanceCm()
{
	return Lidar.lidar.Dist;
}

bool Lidar_isLocked()
{
	return isLocked;
}
