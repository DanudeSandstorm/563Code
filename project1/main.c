#include "stm32l476xx.h"
#include "SysClock.h"
#include "LED.h"
#include "UART.h"
#include "Timer.h"
#include "GPIO.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define CLOCK_SPEED 80000000 // 80 Mhz
#define MAX_LOWER_VAL 9950
#define MIN_LOWER_VAL 50
#define SAMPLES 1000



/* Set LED color to correspond to GPIO value.
   High gives Green
   Low gives Red
*/
void POST() {
	while (1) {
	if (GPIO_Read(GPIOA, 2)) {
		Red_LED_Off();
		Green_LED_On();
	}
	else {
		Red_LED_On();
		Green_LED_Off();
	}
	for(int i = 0; i < 40000; i++);
}
}

void waitForRisingEdge() {
	uint8_t prevValue;
	uint8_t gpioValue = 1; // Initialize to 1 to detect when prevValue becomes 0
	do {
		prevValue = gpioValue;
		gpioValue = GPIOA->IDR & GPIO_IDR_IDR_0;
	} while (!(prevValue == 0 && gpioValue == 1));
}

int main(void){
	char rxByte;
	
	// Default upper and lower bounds for micro second tracking
	int usLowerLim = 950;
	int usUpperLim = usLowerLim + 100;
	
	System_Clock_Init(); // Switch System Clock = 80 MHz
	LED_Init();
	UART2_Init();
	TIM2_Init();
	
	// GPIO Init
	
	GPIOA_Init();
	GPIO_Resistor(GPIOA, 0, GPIO_RES_PULLDOWN);
	GPIO_Mode(GPIOA, 0, GPIO_MODE_INPUT);
	
	GPIO_Resistor(GPIOA, 1, GPIO_RES_PULLDOWN);
	GPIO_Mode(GPIOA, 1, GPIO_MODE_INPUT);
	
	GPIO_Resistor(GPIOA, 2, GPIO_RES_PULLDOWN);
	GPIO_Mode(GPIOA, 2, GPIO_MODE_INPUT);
	
	// Initialize timer with millisecond prescalar
  int prescalar = 79; // Value achieved via magic (re: evesdropping)
	Timer_Prescalar(TIM2, prescalar);
	
	POST();
	
	program_start:
	
	// Show initial limits
	USART_WriteEZ(USART2, "\nLower Limit: %d, Upper limit: %d\r\n", usLowerLim, usUpperLim);
	
	// Block until we get a suitable answer
	do {
		USART_WriteEZ(USART2, "Keep these settings (Y / N):\r\n");
		rxByte = USART_Read(USART2);
	} while (rxByte != 'Y' && rxByte != 'y' && rxByte != 'N' && rxByte != 'n');
	
	// Allow user to change lower threshold when 'N'
	if (rxByte == 'N' || rxByte == 'n') {
		usLowerLim = 0; // Unset this so we don't just skip the following prompt
		
		// Keep reading until we get an allowable value
		while (usLowerLim < MIN_LOWER_VAL || MAX_LOWER_VAL < usLowerLim) {
			USART_WriteEZ(USART2, "Input new lower limit (Value between %d and %d):\r\n", MIN_LOWER_VAL, MAX_LOWER_VAL);
			usLowerLim = USART_ReadInt(USART2);
		}
		
		usUpperLim = usLowerLim + 100;
		
		USART_WriteEZ(USART2, "Limits changed to Lower: %d, Upper: %d\r\n", usLowerLim, usUpperLim);
	}
	
	// Block until enter is pressed
	USART_WriteEZ(USART2, "Press <Enter> key to start\r\n");
	while (USART_Read(USART2) != '\r');
	
	USART_WriteEZ(USART2, "starting\n\r");
	
	Timer_Reset(TIM2);
	Timer_Start(TIM2);
	
	// This loop gets measures the time between 1000 rising edges on a GPIO pin and stores them in a bucket
	static uint16_t bucket[101]; // Our giant-ass bucket of values
	int startTime = 0;
	for (int i = 0; i < 1000; i++) {
		
		waitForRisingEdge();
		startTime = Timer_Read(TIM2);
		waitForRisingEdge();
		
		int timeDiff = Timer_Read(TIM2) - startTime;
		
		// Make sure value is in range
		if (usLowerLim <= timeDiff <= usUpperLim) {
			bucket[timeDiff - usLowerLim]++;
		}
		
		// Give viewer some idea of progress
		USART_WriteEZ(USART2, ".");
		
		// Store the current time for the next loop
		startTime = Timer_Read(TIM2);
	}
	
	USART_WriteEZ(USART2, "\r\n\r\n");
	
	// Print out our nice counts
	for (int i = 0; i < 101; i++) {
		if( bucket[i] != 0) {
			USART_WriteEZ(USART2, "Time %dus count %d\r\n", usLowerLim + i, bucket[i]);
		}
	}
	
	USART_WriteEZ(USART2, "\r\nRun this thing again y/n?\r\n");
	rxByte = USART_Read(USART2);
	
	if(rxByte == 'y' || rxByte == 'Y') {
		goto program_start;
	}
	
	USART_WriteEZ(USART2, "Bye!");
}

