#include "stm32l476xx.h"
#include "UART.h"

#define PWM_PERIOD (200) // 20ms
#define ASCII_0    (48)  // ASCII 48 -> 0
#define ASCII_5    (53)  // ASCII 53 -> 5

void PWM_Init(void);
void setServoPosition(uint8_t position);

/**
 * Reads single-digit values from UART,
 * then moves a servo to reflect those values.
 * Does nothing until the button on the STM board is pressed.
 */
int main(void) {

	PWM_Init();
	UART2_GPIO_Init();

	// TODO wait until button is pressed

	// Display voltage using servo
	while (1) {
		// Block until we read a character
		char input = USART_Read(USART2);

		// Convert valid codes to numbers
		//if (ASCII_0 <= input && input <= ASCII_5) {
			uint8_t voltage = 5;//input - ASCII_0;
			setServoPosition(voltage);
		//}
	}
}

/* Servo PWM Initialize */
void PWM_Init() {
	// Enable TIM2 clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

	/* Started at 8000 and modified until I got 20ms pulses on the
	 * oscilloscope with TIMx->ARR set to 200 */
	TIM2->PSC = 400;         // Set prescalar
	TIM2->EGR |= TIM_EGR_UG; // Trigger timer event to force load prescalar
	
	// Set TIM2 Channel 1 to PWM mode
	TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
	TIM2->CCMR1 |= TIM_CCMR1_OC1PE; // Channel 1 Preload Enable
	
	// Enable auto-reload of preload
	TIM2->CR1 |= TIM_CR1_ARPE;
	
	// Enable channel output bit for TIM2
	TIM2->CCER |= TIM_CCER_CC1E;
	
	// Trigger update to load the above settings
	TIM2->EGR |= TIM_EGR_UG;
	
	// Set period of PWM
	TIM2->ARR = PWM_PERIOD; // TODO consider adjusting this for more resolution
	TIM2->EGR |= TIM_EGR_UG;

	// Start PWM
	TIM2->CR1 |= TIM_CR1_CEN;
}

void setServoPosition(uint8_t position) {

	// TODO potentially pre-calculate these widths
	static double DUTY_CYCLE[] = {
		0.04,   // 0
		0.055,  // 1
		0.0725, // 2
		0.085,  // 3
		0.10,   // 4
		0.12    // 5
	};

	uint32_t pulseWidth = (uint32_t)(DUTY_CYCLE[position] * PWM_PERIOD);

	// Set duty cycle of PWM
	TIM2->CCR1 = pulseWidth;

	// Trigger update to load new pulse
	TIM2->EGR |= TIM_EGR_UG;
}

