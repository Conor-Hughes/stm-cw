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
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // 2.1: Enable SysConfigController to SysClk.

	// Step 2. Configure required pin (PE.9) to be 'alternate function'.
	GPIOE->MODER &= ~(0xFF000); // Reset PE6 -> PE9
	GPIOE->MODER |= 0xA0000; // Set PE8 and PE9 to alternate functions.
	GPIOE->OTYPER &= ~(0x00000100); // Set output type for each pin required in Port E
	GPIOE->PUPDR &= ~(0x00000000); // Set Pull up/Pull down resistor configuration for Port E

	// Step 3: Set up alternate function for PE8 & PE9
	GPIOE->AFR[1] |= 0x22; // Set PE8 & PE9 to receive input from TIM1_CH1.

	// Step 4: Initialise timer with PSC and ARR:
	TIM1->PSC = 999999999; // TODO: Maybe need to change this based on the CCHR value.
	TIM1->ARR = 199;
	
	// Step 5: Set Timer 1 Channel 1 to be in PWM mode.
	//TIM1->CCMR1 |= 0x00000060;
	TIM1->CCMR1 |= 0xF0;
	TIM1->CCMR2 |= 0xF0;
	
	// Step 6: Set the CCR1 to an initial value ("on time" of PWM pulse).
	TIM1->CCR1 = 100; // Sets on time to 10 clock pulses
	TIM1->CCR2 = 50; // Sets on time to 10 clock pulses
	
	// Step 7: Enable the channel chosen to be the output to the GPIO pin.
	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->CCER |= TIM_CCER_CC1E;
	TIM1->CCER |= TIM_CCER_CC1NE;
	TIM1->CCER |= 0x8; // Set the channel CH1N to be active-low (to mirror CH1).
	
	// Remove the lock from the register:
	TIM1->BDTR &= ~(0x300);
	// Add the max dead-time to the timer.
	TIM1->BDTR |= 0xE0;
	
	// 8. Enable the timer.
	TIM1->CR1 |= TIM_CR1_CEN; // Enables the timer.
	
	// 2.2: Remove mask to enable an interrupt to be generated from the EXTI_IMR register
	EXTI->IMR |= EXTI_IMR_MR0;
	
	// 2.3: Set interrupt trigger to be rising edge:
	EXTI->RTSR |= EXTI_RTSR_TR0;
	
	// 2.4: Configure multiplexing options to enable PA.0 to generate interrupt EXTIO.
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA;
	
	// 2.5 Configure the NVIC to trigger the ISR.
	NVIC_EnableIRQ(EXTI0_IRQn);
	

	while(1){
	}

}

int levels[] = {0, 25, 50, 75, 100};
int i = 1;

void EXTI0_IRQHandler()
{
	if (EXTI->PR & EXTI_PR_PR0) // check source
	{
		EXTI->PR |= EXTI_PR_PR0; // clear flag*
		TIM1->CCR1 = (levels[i] / 10);
		
		i++;
		if(i > 3){
			i = 0;
		}
	}
};
