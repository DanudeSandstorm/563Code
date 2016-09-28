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
	
	// Started at 8000 and modified until I got 20ms pulses on the oscilloscope with TIMx->ARR set to 200
	Timer_Prescalar(TIM2, 400);
	
	// Tell GPIOA pins 0 and 1 to be in Alternate Function mode
	GPIOA->MODER &= ~(0xFFFF); // Unset MODER
	GPIOA->MODER |= 0x000A;
	
	//Connect GPIOA pins 0 and 1 to TIM2 via alternate function mode
	GPIOA->AFR[0] |= 0x0011;
	
	// Set TIM2 Channels 1 and 2 to PWM mode
	TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // Channel 1
	TIM2->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1; // Channel 2
	TIM2->CCMR1 |= TIM_CCMR1_OC1PE; // Channel 1 Preload Enable
	TIM2->CCMR1 |= TIM_CCMR1_OC2PE; // Channel 2 Preload Enable
	
	// Enable auto-reload of preload
	TIM2->CR1 |= TIM_CR1_ARPE;
	
	// Enable channel output bit for TIM2
	TIM2->CCER |= TIM_CCER_CC1E;	// Channel 1
	TIM2->CCER |= TIM_CCER_CC2E;	// Channel 2
	
	// Trigger update to load the above settings
	TIM2->EGR |= TIM_EGR_UG;
	
	// Set period of PWM
	TIM2->ARR = 200; // 20ms
	TIM2->EGR |= TIM_EGR_UG;
	
	// Set pulse of PWM
	TIM2->CCR1 = 20; // 2ms
	TIM2->CCR2 = 20;
	TIM2->EGR |= TIM_EGR_UG;
	
	// Start PWM
	TIM2->CR1 |= TIM_CR1_CEN;
	
	// Loop indefinitely while the timer does its thing
	while (1);
}
