#include "stm32l476xx.h"
#include "UART.h"
#include "GPIO.h"
#include "Timer.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void interpretCommands(char input) {
	switch (input) {
		// Pause recipe execution
		case 'P':
		case 'p':
			// TODO
			break;
		
		// Continue recipe execution
		case 'C':
		case 'c':
			// TODO
			break;
		
		// Move 1 position to right, if possible
		case 'R':
		case 'r':
			// TODO
			break;
		
		// Move 1 position to left, if possible
		case 'L':
		case 'l':
			// TODO
			break;
		
		// No-op
		case 'N':
		case 'n':
			// TODO
			break;
		
		// Restart recipe
		case 'B':
		case 'b':
			// TODO
			break;
	}
}


int main(void) {

	TIM2_Init();
	GPIOA_Init();
	
	// TODO actually get this prescalar in a good way
	Timer_Prescalar(TIM2, 100); // 100 us / count
	
	// Connect GPIO pins to TIM2 via alternate function mode
	GPIO_Mode(GPIOA, 0, GPIO_MODE_ALTERNATE);
	GPIO_Mode(GPIOA, 1, GPIO_MODE_ALTERNATE);
	
	
	
	// Set the alternate function register to the timer (see datasheet)
	// We can use the same timer with 2 channels
	
	// GPIOA PA0 AF1 = TIM2_CH1
	// GPIOA PA1 AF1 = TIM2_CH2
	GPIOA->AFR |= GPIO_AFRL_AFRL1;
	
	
	
	
	
	
	// Init gpio (enable clocks, etc)
	// Tie gpio to tim2 with alternate function
	
	// Set tim2 prescaler to 100 us
	// Set tim2 period to 200
	// Set mode to output - PWM (edge mode)
	
	// use compare 1 for gpio1/servo 1
	// Use compare 2 for gpio2
	
	// Adjust compare values from 4 to 20 to change the width, and thus position servos

	TIM2->CCMR1
	
	return 0;
}
