/* 
	Pete Hubbard 2019
	Loughborough University
	WSC055 Lab 1
	V.2.0
	
	The following 'c' code presents an outline for you to adapt during the laboratory
	
	*/

#include "stm32f3xx.h"                  // Device header

int i = 0;

int main(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOEEN;	// Enable clock on GPIO port E
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // Direct clock pulses to timer 3

	/**
	*	Setting values for PSC and ARR so that the timer will send an interrupt signal every 1s.
	*/
	TIM3->PSC = 100;
	TIM3->ARR = 7999;
	TIM3->CR1 |= TIM_CR1_CEN; // Enables the timer.
	
	// GPIOE is a structure defined in stm32f303xc.h file
	// Define settings for each output pin using GPIOE structure
	GPIOE->MODER |= 0x55550000; // Set mode of each pin in port E
	GPIOE->OTYPER &= ~(0x00000100); // Set output type for each pin required in Port E
	GPIOE->PUPDR &= ~(0x00000000); // Set Pull up/Pull down resistor configuration for Port E
	
	TIM3->DIER |= TIM_DIER_UIE; // Set DIER register to watch out for an ‘Update’ Interrupt Enable (UIE) – or 0x00000001
	NVIC_EnableIRQ(TIM3_IRQn); // Enable Timer ‘x’ interrupt request in NVIC
			
	while (1)
  {
		
	}

}

void TIM3_IRQHandler()
{
	if ((TIM3->SR & TIM_SR_UIF) !=0) // Check interrupt source is from the ‘Update’ interrupt flag
	{
		
		// Turn off all previous LEDS:
		GPIOE->BSRRH = (i - 1) << 8;
		
		// Reset the counter once it reaches the full value.
		if(i > 255) {
			i = 0;
		}
	
		GPIOE->BSRRL = i << 8;
		i++;
	}
	
	TIM3->SR &= ~TIM_SR_UIF; // Reset ‘Update’ interrupt flag in the SR register
}
