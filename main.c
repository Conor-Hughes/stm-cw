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
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; // Direct clock pulses to Timer 1
	RCC->AHBENR |= RCC_AHBENR_GPIOEEN;	// Enable clock on GPIO port E

	// Step 2. Configure required pin (PE.9) to be 'alternate function'.
	GPIOE->MODER &= ~(0xC0000);
	GPIOE->MODER |= 0x80000; // Set mode of each pin in port E
	GPIOE->OTYPER &= ~(0x00000100); // Set output type for each pin required in Port E
	GPIOE->PUPDR &= ~(0x00000000); // Set Pull up/Pull down resistor configuration for Port E

	// Step 3: Set up alternate function for PE.9:
	GPIOE->AFR[1] |= 0x20; // Set PE.9 to receive input from TIM1_CH1.

	// Step 4: Initialise timer with PSC and ARR:
	TIM1->PSC = 7999; // TODO: Maybe need to change this based on the CCHR value.
	TIM1->ARR = 9;
	
	// Step 5: Set Timer 1 Channel 1 to be in PWM mode.
	TIM1->CCMR1 |= 0x00000060;
	
	// Step 6: Set the CCR1 to an initial value ("on time" of PWM pulse).
	TIM1->CCR1 = 10; // Sets on time to 10 clock pulses
	
	// Step 7: Enable the channel chosen to be the output to the GPIO pin.
	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->CCER |= TIM_CCER_CC1E;
	
	// 8. Enable the timer.
	TIM1->CR1 |= TIM_CR1_CEN; // Enables the timer.
	

	while(1){
	}

}
