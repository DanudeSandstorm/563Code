#include "GPIO.h"

// Mask that zeros the two bits that correspond to the given pin
#define pinModeMask(pin, mode) (0xFFFF & (0xC << (pin * 2)) | (mode << (pin * 2)) )

/* Enable GPIOA clock */
void GPIOA_Init(void) {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
}

/* Enable GPIOB clock */
void GPIOB_Init(void) {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
}

/* Sets a GPIO pin's resistor to pull-up or pull-down mode */
void GPIO_Resistor(GPIO_TypeDef * GPIOx, uint8_t pin, uint8_t resmode) {
	GPIOx->PUPDR |= resmode << (pin * 2); // Move setting to appropriate memory position
}

/* Sets a GPIO pin's mode */
// FIXME this breaks the board
void GPIO_Mode(GPIO_TypeDef * GPIOx, uint8_t pin, uint8_t pinmode) {
	GPIOx->MODER &= pinModeMask(pin, pinmode);
}

uint8_t GPIO_Read(GPIO_TypeDef * GPIOx, uint8_t pin) {
	return ((GPIOx->IDR >> pin) & 0x1);
}
