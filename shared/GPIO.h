#ifndef __STM32L476G_DISCOVERY_GPIO_H
#define __STM32L476G_DISCOVERY_GPIO_H

#include "stm32l476xx.h"

#define PIN_0 (0)
#define PIN_1 (1)
#define PIN_2 (2)
#define PIN_3 (3)
#define PIN_4 (4)
#define PIN_5 (5)
#define PIN_6 (6)
#define PIN_7 (7)

void GPIOA_Init(void);
void GPIOB_Init(void);

#define GPIO_RES_PULLDOWN 0x2
#define GPIO_RES_PULLUP   0x1
#define GPIO_RES_NONE     0x0
void GPIO_Resistor(GPIO_TypeDef * GPIOx, uint8_t pin, uint8_t resmode);


#define GPIO_MODE_INPUT     0x1 // Data sheet appears to be wrong about these values
#define GPIO_MODE_OUTPUT    0x0
#define GPIO_MODE_ALTERNATE 0x2
void GPIO_Mode(GPIO_TypeDef * GPIOx, uint8_t pin, uint8_t pinmode);
void GPIO_ResetMode(GPIO_TypeDef * GPIOx);

uint8_t GPIO_Read(GPIO_TypeDef * GPIOx, uint8_t pin);


#endif /* __STM32L476G_DISCOVERY_GPIO_H */
