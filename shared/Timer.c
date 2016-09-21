#include "Timer.h"

void TIM2_Init(void) {
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;	// Enable TIM2 clock

}

void Timer_Prescalar(TIM_TypeDef * TIMx, int prescalar) {
	TIMx->PSC = prescalar;		// Set prescalar
	TIMx->EGR |= TIM_EGR_UG;	// Trigger timer event to force load prescalar
}

void Timer_Reset(TIM_TypeDef * TIMx) {

}

void Timer_Start(TIM_TypeDef * TIMx) {
	TIMx->CR1 |= 1;	// Tell Timer to start
}


int Timer_Read(TIM_TypeDef * TIMx) {
	return TIMx->CNT;	// Grabs the value out of the counter register
}

//void Timer_SetMode(TIM_TypeDef * TIMx, );
