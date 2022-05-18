/* 
	Pete Hubbard 2019
	Loughborough University
	WSC055 Lab 1
	V.2.0
	
	The following 'c' code presents an outline for you to adapt during the laboratory
	
	*/

#include "stm32f3xx.h"                  // Device header
int Count = 1 ;
int Ratio[] = {1,5,9};
int indexer = 0; 
const int StartConfig[10][2] = {{1,0},{1,1},{0,1},{0,0},{1,0},{1,1},{0,1},{0,0},{1,0},{1,1}};
const int EncoderSig[2][5][2] = {{{1,0},{1,1},{0,1},{0,0},{1,0}},{{1,1},{0,1},{0,0},{1,0},{1,1}}};

void signalfunc(){
	
}

int main(void)
{
	// Enable clock on GPIO port E
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
	
	GPIOE->MODER |= (0xA0000); 
	GPIOE->OTYPER &= ~(0x00000100);
	GPIOE->PUPDR &= ~(0x00000000); 
	
	GPIOE->AFR[1]|=0x00000022;
	
	TIM1->PSC = 39999;
	TIM1->ARR = 99;
	
	TIM1->CCMR1 |= 0x1A0F0;
	
	
	TIM1->CCR1 = 0;
	TIM1->CCR2 = 25; 
	
	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->CCER |= TIM_CCER_CC1E;
	//TIM1->CCER |= TIM_CCER_CC2E;
	TIM1->CR1|=TIM_CR1_CEN;
	

	
	EXTI->IMR |= EXTI_IMR_MR0;
	EXTI->RTSR|= EXTI_RTSR_TR0;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA;
	NVIC_EnableIRQ(EXTI0_IRQn);
	
	
	

	

	
	// Main programme loop - make LED 4 (attached to pin PE.0) turn on and off	
	while (1)
  {
				// Reset the counter once it reaches the full value.
		//GPIOE->BSRRL = 0xFF00; // From pins 8 -> 15
		//delay(600000);
		//GPIOE->BSRRH = 0xFF00;
		//delay(600000);
	}

}




void EXTI0_IRQHandler(){
	if (EXTI->PR & EXTI_PR_PR0) // check source
	{
	EXTI->PR |= EXTI_PR_PR0; // clear flag*
	if (Count < 3){
		Count ++ ;
	}
	if (Count > 2){
		Count = 0;
	}
	TIM1->CCR1 = Ratio[Count]; 
	}
}



