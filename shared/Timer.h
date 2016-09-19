#ifndef __STM32L476G_DISCOVERY_TIMER_H
#define __STM32L476G_DISCOVERY_TIMER_H

#include "stm32l476xx.h"

void TIM2_Init(void);
void Timer_Prescalar(TIM_TypeDef * TIMx, int prescalar);
void Timer_Reset(TIM_TypeDef * TIMx);
void Timer_Start(TIM_TypeDef * TIMx);
int Timer_Read(TIM_TypeDef * TIMx);

#endif /* __STM32L476G_DISCOVERY_TIMER_H */
