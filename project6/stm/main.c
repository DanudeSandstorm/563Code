#include "stm32l476xx.h"
#include "GPIO.h"
#include "Timer.h"

#define PWM_PRESCALAR (200)
#define PWM_PERIOD    (400) // 20ms
#define MIN_VOLTAGE   (-5)
#define MAX_VOLTAGE   (5)

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
	/*
	GPIOA_Init();
	GPIO_Resistor(GPIOA, PIN_0, GPIO_RES_PULLDOWN);
	GPIO_Resistor(GPIOA, PIN_1, GPIO_RES_PULLDOWN);
	GPIO_Resistor(GPIOA, PIN_2, GPIO_RES_PULLDOWN);
	GPIO_Resistor(GPIOA, PIN_3, GPIO_RES_PULLDOWN);
	GPIO_Mode(GPIOA, PIN_0, GPIO_MODE_INPUT);
	GPIO_Mode(GPIOA, PIN_1, GPIO_MODE_INPUT);
	GPIO_Mode(GPIOA, PIN_2, GPIO_MODE_INPUT);
	GPIO_Mode(GPIOA, PIN_3, GPIO_MODE_INPUT);
*/
	
	// TODO wait until button is pressed
	while (1);
	
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
	
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN; // Enable TIM2 clock

	// TIM2 timing setup
	TIM2->PSC = PWM_PRESCALAR;            // Set prescalar
	TIM2->ARR = PWM_PERIOD;               // Set period TODO consider adjusting this for more resolution
	
	// TIM2 channel setup
	TIM2->CCMR1 |= TIM_CCMR1_OC1M_1;      // Set TIM2_CH1 to output compare mode
	TIM2->CCMR1 |= TIM_CCMR1_OC1M_2;
	TIM2->CCMR1 |= TIM_CCMR1_OC1PE;       // Set TIM2_CH1 output compare Preload Enable
	TIM2->CR1 |= TIM_CR1_ARPE;            // Enable auto-reload of preload
	TIM2->CCER |= TIM_CCER_CC1E;          // Enable channel output bit for TIM2_CH1
	
	// Finalize
	TIM2->EGR |= TIM_EGR_UG;              // Trigger update to load the above settings into TIM2

	// GPIO Alternate Function mode for PWM output
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;  // Enable GPIOA
	GPIOA->MODER &= ~GPIO_AFRL_AFRL0;     // Clear MODER
	GPIOA->MODER |= GPIO_MODER_MODER0_1;  // Set GPIOA to Alternate Function mode
	GPIOA->AFR[0] |= 0x01;                // TIM2_CH1 output on GPIO pin PA0

	// Start PWM
	TIM2->CR1 |= TIM_CR1_CEN;             // TIM2 Counter enable
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

