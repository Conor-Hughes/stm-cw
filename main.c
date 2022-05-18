/* 
	Pete Hubbard 2019
	Loughborough University
	WSC055 Lab 1
	V.2.0
	
	The following 'c' code presents an outline for you to adapt during the laboratory
	
	*/

#include "stm32f3xx.h"                  // Device header
typedef enum {false, true} bool;

int indexer = 0;
bool clockwise = true;
const int signals[8][2] = {{0,1}, {0,0}, {1,0}, {1,1}, {0,1}, {0,0}, {1,0}, {1,1}};

void setup_timer_3();
void setup_port_e();
	
int main(void)
{
	
	setup_timer_3();
	
	setup_port_e();
	
	//RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // Enable the interrupt on the button.
			
	
	/**
		EXTI->IMR |= EXTI_IMR_MR0;
		EXTI->RTSR|= EXTI_RTSR_TR0;
		SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA;
		NVIC_EnableIRQ(EXTI0_IRQn);
	*/
	
	while (1)
  {
	}
}

void setup_timer_3(){	
	
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // Direct clock pulses to timer 3
	
	/**
	*	Setting values for PSC and ARR so that the timer will send an interrupt signal every 1s.
	*/
	TIM3->PSC = 1000;
	TIM3->ARR = 79990;
	
	TIM3->CR1 |= TIM_CR1_CEN; // Enables the timer.
	
	TIM3->DIER |= TIM_DIER_UIE; // Set DIER register to watch out for an ‘Update’ Interrupt Enable (UIE) – or 0x00000001
	NVIC_EnableIRQ(TIM3_IRQn); // Setup the ISR for the timer.
}

void setup_port_e(){
	RCC->AHBENR |= RCC_AHBENR_GPIOEEN; // Enable clock on GPIO port E
	
	GPIOA->MODER &= ~(0xFFFFF000); // Set PE 15 -> 6 as Inputs.
	GPIOE->MODER |= (0x55550000); // Set Pe 15 -> 8 as GP Outputs. 
	GPIOE->OTYPER &= ~(0x00000100); // Set output type for each pin required in Port E
	GPIOE->PUPDR &= ~(0x00000000); // Set Pull up/Pull down resistor configuration for Port E
}

int i = 0;

/*
*		Generates the square wave on both channels.
*/
void TIM3_IRQHandler()
{
	if ((TIM3->SR & TIM_SR_UIF) !=0) // Check interrupt source is from the ‘Update’ interrupt flag
	{
		
		if(clockwise){
			indexer = indexer + 1;
			
			// If indexer has overflown, reset it back to 0.
			if(indexer > 7){
				indexer = 0;
			}
		}
		else {
			indexer = indexer - 1;
			
			// If indexer has gone negative, reset it to the max value:
			if(indexer < 0){
				indexer = 7;
			}
		}
		
		// Get the next value of channel A and B:
		int channelAValue = signals[indexer][0]; // Channel A = PE8
		int channelBValue = signals[indexer][1]; // Channel B = PE9
		
		if(channelAValue == 1){
			GPIOE->BSRRL |= 0x100;
		}
		else {
			GPIOE->BSRRH |= 0x100;
		}
		
		if(channelBValue == 1){
			GPIOE->BSRRL |= 0x200;
		}
		else {
			GPIOE->BSRRH |= 0x200;
		}
	}
	
	TIM3->SR &= ~TIM_SR_UIF; // Reset ‘Update’ interrupt flag in the SR register
}

void EXTI0_IRQHandler(){
	if (EXTI->PR & EXTI_PR_PR0) // check source
	{
		EXTI->PR |= EXTI_PR_PR0; // clear flag*
		
		if(indexer == 0){
			indexer = 1;
		}
		else {
			indexer = 0;
		}
	}
}



