#ifndef __STM32L476G_DISCOVERY_GPIO_H
#define __STM32L476G_DISCOVERY_GPIO_H

#include "stm32l476xx.h"

void GPIOA_Init(void);

#define GPIO_RES_PULLDOWN 0x2
#define GPIO_RES_PULLUP   0x1
#define GPIO_RES_NONE     0x0
void GPIO_Resistor(GPIO_TypeDef * GPIOx, uint8_t pin, uint8_t resmode);


#define GPIO_MODE_INPUT     0x0
#define GPIO_MODE_OUTPUT    0x1
#define GPIO_MODE_ALTERNATE 0x2
void GPIO_Mode(GPIO_TypeDef * GPIOx, uint8_t pin, uint8_t pinmode);

//void GPIO_Read(GPIO_TypeDef * GPIOx, );


#endif /* __STM32L476G_DISCOVERY_GPIO_H */
