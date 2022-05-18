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
void setup_button_interrupt();
void setup_pe_7_and_8_interrupt();
void increment_counter();
void decrement_counter();
void set_counter();
	
int main(void)
{
	
	setup_timer_3();
	
	setup_port_e();
	
	setup_button_interrupt();
	
	setup_pe_7_and_8_interrupt();
	
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

void setup_button_interrupt(){
	
	// 2.1: Enable SysConfigController to SysClk.
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	
	// 2.2: Remove mask to enable an interrupt to be generated from the EXTI0_IMR register
	EXTI->IMR |= EXTI_IMR_MR0;
	
	// 2.3: Set interrupt trigger to be rising edge:
	EXTI->RTSR |= EXTI_RTSR_TR0;
	
	// 2.4: Configure multiplexing options to enable PA.0 to generate interrupt EXTIO.
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA;
	
	// 2.5 Configure the NVIC to trigger the ISR.
	NVIC_EnableIRQ(EXTI0_IRQn);
}

void setup_pe_7_and_8_interrupt()
{
	// Remove masks to enable interrupts from EXTI6 & EXTI7:
	EXTI->IMR |= EXTI_IMR_MR6;
	EXTI->IMR |= EXTI_IMR_MR7;
	
	// Set EXTI6 & EXTI7 ports to generate interrupt on rising and falling edges:
	EXTI->RTSR |= EXTI_RTSR_TR6;
	EXTI->RTSR |= EXTI_RTSR_TR7;
	EXTI->FTSR |= EXTI_FTSR_TR6;
	EXTI->FTSR |= EXTI_FTSR_TR7;
	
	// Configure multiplexing options to enable PE.6 and PE.7 to generate interrupt EXTIO.
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PE;
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI7_PE;
	
	NVIC_EnableIRQ(EXTI9_5_IRQn);
}

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
		
		// Flip the direction of the encoder signal.
		clockwise = !clockwise;
	}
}
	
void EXTI9_5_IRQHandler(){
		
	/*
	* Channel A - PE8. Wired to PE7.
	*	When this is rising, check to see if Channel B is low. If so, we're going clockwise.
	*	When this is falling, check to see if Channel B is high. If so, we're going clockwise.
	*/
	if (EXTI->PR & EXTI_PR_PR7) // check source
	{
		EXTI->PR |= EXTI_PR_PR7; // clear flag
		
		// If this was a rising edge:
		if(GPIOE->IDR & (1<<7)){
						
			// Check if Channel B is low. If it is, we can increment by 1. Else, decrement 1.
			if(GPIOE->IDR & (1<<6)){
				decrement_counter();
			}
			else {
				increment_counter();
			}
		}
		// Else, this is a falling edge:
		else {
			
			// Check if Channel B is high. If it is, we can increment by 1. Else, decrement 1.
			if(GPIOE->IDR & (1<<6)){
				increment_counter();
			}
			else {
				decrement_counter();
			}
			
		}
	}
	
	/*
	* Channel B - PE9. Wired to PE6.
	*	When this is rising, check to see if Channel A is high. If so, we're going clockwise.
	*	When this is falling, check to see if Channel A is low. If so, we're going clockwise.
	*/
	if (EXTI->PR & EXTI_PR_PR6) // check source
	{
		EXTI->PR |= EXTI_PR_PR6; // clear flag*
		
		// If this was a rising edge...
		if(GPIOE->IDR & (1<<6)){
			
			// Check if Channel A is high. If it is, we can increment by 1. Else, decrement 1.
			if(GPIOE->IDR & (1<<7)){
				increment_counter();
			}
			else {
				decrement_counter();
			}
			
		}
		// If this was a falling edge...
		else {
			
			// Check if Channel A is high. If it is, we can increment by 1. Else, decrement 1.
			if(GPIOE->IDR & (1<<7)){
				decrement_counter();
			}
			else {
					increment_counter();
			}

		}
	}
}

int encoderCount = 0;

void adjust__counter(bool add){
	
	if(add){
		encoderCount++;
	}
	else {
		encoderCount--;
	}
	
	
	if(encoderCount > 15){
		encoderCount = 0;
	}
	
	// Reset all LEDS from PE11 -> PE14.
	GPIOE->BSRRH &= ~(0x7800);
	
	GPIOE->BSRRL |= encoderCount << 11;
}

void increment_counter(){
	encoderCount++;
	
	if(encoderCount > 15){
		encoderCount = 15;
	}
	
	set_counter();
}

void decrement_counter(){
	encoderCount--;
	
	if(encoderCount < 0){
		encoderCount = 0;
	}
	
	set_counter();
}

void set_counter(){
	// Reset all LEDS from PE11 -> PE14.
	GPIOE->BSRRH |= 0x7800;
	
	GPIOE->BSRRL |= encoderCount << 11;
}




