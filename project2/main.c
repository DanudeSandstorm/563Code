#include "stm32l476xx.h"
#include "UART.h"
#include "GPIO.h"
#include "LED.h"
#include "Timer.h"
#include "task.h"
#include "recipes.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define SERVO_SCALAR 5

void setErrorState(void);
void setServoPosition(uint8_t, uint8_t);

uint8_t servo1Position = 0;
uint8_t servo2Position = 0;

Task servoTask1;
Task servoTask2;

void interpretCommands(char input) {
	switch (input) {
		// Pause recipe execution
		case 'P':
		case 'p':
			servoTask1.status |= STATUS_PAUSED;
			servoTask2.status |= STATUS_PAUSED;
			break;
		
		// Continue recipe execution
		case 'C':
		case 'c':
			servoTask1.status &= ~STATUS_PAUSED;
			servoTask2.status &= ~STATUS_PAUSED;
			break;
		
		// Move 1 position to right, if possible
		case 'R':
		case 'r':
			setServoPosition(1, servo1Position + 1);
			setServoPosition(2, servo2Position + 1);
			break;
		
		// Move 1 position to left, if possible
		case 'L':
		case 'l':
			setServoPosition(1, servo1Position - 1);
			setServoPosition(2, servo2Position - 1);
			break;
		
		// No-op
		case 'N':
		case 'n':
			// TODO
			break;
		
		// Restart recipe
		case 'B':
		case 'b':
			servoTask1.ip = 0;
			servoTask1.status = 0;
			servoTask2.ip = 0;
			servoTask2.status = 0;
			break;
	}
}

void stepTask(Task *task) {

	// Check for delays
	if (task->waitCount) {
		task->waitCount--;
		return;
	}

	// Check for paused/done
	if (task->status & (STATUS_PAUSED | STATUS_END)) {
		return;
	}

	// Decode current operation for recipe
	char op, arg;
	arg =  *(task->ip) & 0x1F; // Last 5 bits contain argument for command
	op = ( *(task->ip) >> 5) & 0x7; // Most significant 3 bits contain operation code

	task->ip++; // Increment IP

	switch (op) {
		case OP_MOV:
			setServoPosition(task->servoId, arg);
			break;

		case OP_WAIT:
			task->waitCount = arg;
			break;

		case OP_LOOP_START:
			task->loopCount = arg;
			task->loopStart = task->ip;
			break;

		case OP_END_LOOP:
			if (task->loopCount) {
				task->loopCount--;
				task->ip = task->loopStart;
			} else {
				task->loopStart = NULL;
			}

			break;

		case OP_RECIPE_END:
			task->status |= STATUS_END;
			break;
	}
}
void setServoPosition(uint8_t servo, uint8_t position) {
	// Handle out of bounds values
	if (position > 5) {
		setErrorState();
		return;
	}

	uint8_t pulseWidth = position * SERVO_SCALAR;

	// Set duty cycle of PWM
	if (servo == 1) {
		servo1Position = position;
		TIM2->CCR1 = pulseWidth;
	}

	if (servo == 2) {
		servo2Position = position;
		TIM2->CCR2 = pulseWidth;
	}

	TIM2->EGR |= TIM_EGR_UG;
}

void setErrorState() {
	Red_LED_On();
}

int main(void) {

	TIM2_Init();
	GPIOA_Init();
	LED_Init();
	
	servoTask1 = createTask(demo1, 1);
	servoTask2 = createTask(demo1, 2);

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
	
	
	setServoPosition(1, 0);
	setServoPosition(2, 0);

	// Start PWM
	TIM2->CR1 |= TIM_CR1_CEN;
	

	// Loop indefinitely while the timer does its thing
	while (1) {
		stepTask(&servoTask1);
		stepTask(&servoTask2);
	}
}
