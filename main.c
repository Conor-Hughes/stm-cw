/* 
	Pete Hubbard 2019
	Loughborough University
	WSC055 Lab 1
	V.2.0
	
	The following 'c' code presents an outline for you to adapt during the laboratory
	
	*/

#include "stm32f3xx.h"                  // Device header


int main(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOEEN;	// Enable clock on GPIO port E
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; // Direct clock pulses to Timer 1
	GPIOE->AFR[0]|=0x00000200;

	/**
	*	Setting values for PSC and ARR so that the timer will send an interrupt signal every 1s.
	*/
	TIM1->PSC = 7999; // TODO: Maybe need to change this based on the CCHR value.
	TIM1->ARR = 9;
	
	TIM1->CCMR1 |= 0x00000060; // Set channel 1 to be in standard PWM mode.
	TIM1->CCR1 = 10; //Sets on time to 10 clock pulses
	
	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->CCER |= TIM_CCER_CC1E;
	
	TIM1->CR1 |= TIM_CR1_CEN; // Enables the timer.
		
	while(1){
	
	}

}

void TIM3_IRQHandler()
{
	if ((TIM3->SR & TIM_SR_UIF) !=0) // Check interrupt source is from the ‘Update’ interrupt flag
	{
		
	}
	
	TIM3->SR &= ~TIM_SR_UIF; // Reset ‘Update’ interrupt flag in the SR register
}
