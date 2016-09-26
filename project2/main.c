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
	Timer_Prescalar(TIM2, 0x1F40); // 100 us / count
	
	// Tell GPIOA pins 0 and 1 to be in Alternate Function mode
	//GPIO_Mode(GPIOA, 0, GPIO_MODE_ALTERNATE);
	//GPIO_Mode(GPIOA, 1, GPIO_MODE_ALTERNATE);
	GPIOA->MODER &= ~(0xFFFF); // Unset MODER
	GPIOA->MODER |= 0x000A;
	
	//Connect GPIOA pins 0 and 1 to TIM2 via alternate function mode
	GPIOA->AFR[0] |= 0x0011;
	
	// Set TIM2 to PWM mode
	TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
	TIM2->CCMR1 |= TIM_CCMR1_OC1PE; // Set preload enable
	
	// Enable auto-reload of preload
	TIM2->CR1 |= TIM_CR1_ARPE;
	
	// Enable channel output bit for TIM2
	TIM2->CCER |= TIM_CCER_CC1E;
	
	// Trigger update to load the above settings
	TIM2->EGR |= TIM_EGR_UG;
	
	
	// Set period of PWM
	TIM2->ARR = 200; // 200 cycles
	TIM2->EGR |= TIM_EGR_UG;
	
	// Set pulse of PWM
	TIM2->CCR1 = 5;
	TIM2->EGR |= TIM_EGR_UG;
	
	// Start PWM
	TIM2->CR1 |= TIM_CR1_CEN;
	
	// Loop indefinitely while the timer does its thing
	while (1);
	// Init gpio (enable clocks, etc)
	// Tie gpio to tim2 with alternate function
	
	// Set tim2 prescaler to 100 us
	// Set tim2 period to 200
	// Set mode to output - PWM (edge mode)
	
	// use compare 1 for gpio1/servo 1
	// Use compare 2 for gpio2
	
	// Adjust compare values from 4 to 20 to change the width, and thus position servos

}
