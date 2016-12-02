#include "stm32l476xx.h"
#include "GPIO.h"
#include "Timer.h"

#define PWM_PERIOD  (200) // 20ms
#define MIN_VOLTAGE (-5)
#define MAX_VOLTAGE (5)

void PWM_Init(void);
int8_t readVoltage(void);
void setServoPosition(uint8_t position);

/**
 * Reads single-digit values from UART,
 * then moves a servo to reflect those values.
 * Does nothing until the button on the STM board is pressed.
 */
int main(void) {

	// PWM and GPIO setup
	PWM_Init();
	GPIOA_Init();
	GPIO_Resistor(GPIOA, PIN_0, GPIO_RES_PULLDOWN);
	GPIO_Resistor(GPIOA, PIN_1, GPIO_RES_PULLDOWN);
	GPIO_Resistor(GPIOA, PIN_2, GPIO_RES_PULLDOWN);
	GPIO_Resistor(GPIOA, PIN_3, GPIO_RES_PULLDOWN);
	GPIO_Mode(GPIOA, PIN_0, GPIO_MODE_INPUT);
	GPIO_Mode(GPIOA, PIN_1, GPIO_MODE_INPUT);
	GPIO_Mode(GPIOA, PIN_2, GPIO_MODE_INPUT);
	GPIO_Mode(GPIOA, PIN_3, GPIO_MODE_INPUT);

	// TODO wait until button is pressed

	// Display voltage using servo
	while (1) {
		int8_t voltage = readVoltage();

		// Validate voltage is within range
		if (MIN_VOLTAGE <= voltage && voltage <= MAX_VOLTAGE) {
			setServoPosition(voltage - MIN_VOLTAGE);
		}
	}
}

/* Reads the voltage encoded on the GPIOA pins.
 *
 * Pin 3 is the sign bit.
 * Pins 2, 1, and 0 represent the voltage value.
 */
int8_t readVoltage() {
	int8_t voltage;

	// Set the sign bit
	if (GPIO_Read(GPIOA, PIN_3)) {
		voltage = 0x80;
	} else {
		voltage = 0x00;
	}

	// Read the value in
	voltage |= GPIO_Read(GPIOA, PIN_2) << 2;
	voltage |= GPIO_Read(GPIOA, PIN_1) << 1;
	voltage |= GPIO_Read(GPIOA, PIN_0);

	return voltage;
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

