/* 
	Pete Hubbard 2019
	Loughborough University
	WSC055 Lab 1
	V.2.0
	
	The following 'c' code presents an outline for you to adapt during the laboratory
	
	*/

#include "stm32f3xx.h"                  // Device header
#include <math.h>

typedef enum {false, true} bool;

void configure_dac();
void configure_timer_3();
void configure_leds();
void configure_adc();
void delay_ten_microseconds();

	
int main(void)
{
	
	configure_timer_3();
	configure_leds();
	configure_dac();
	configure_adc();
	
	while (1){
		// Set the ADSTART bit high to start the conversion:
		ADC1->CR |= 0x4;
		
		if(ADC1->ISR & 0x4) {
			// Turn off all previous LEDS:
			GPIOE->BSRRH = 31 << 11;
			
			// We need 7 bits to represent the maximum number given by the triangle wave
			
			// Let's first check we are correctly receiving the values.
			
			// Divide our number by 50 to get the throttle position in degrees:
			int degrees = (int)round(ADC1->DR / 50); 
			GPIOE->BSRRL = 255 << 8;
		}
	}
	
}

void configure_timer_3()
{
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // Direct clock pulses to timer 3
		TIM3->PSC = 1000;
		TIM3->ARR = 79990;
	
		TIM3->CR1 |= TIM_CR1_CEN; // Enables the timer.
	
		TIM3->DIER |= TIM_DIER_UIE; // Set DIER register to watch out for an ‘Update’ Interrupt Enable (UIE) – or 0x00000001
		NVIC_EnableIRQ(TIM3_IRQn); // Enable Timer ‘x’ interrupt request in NVIC
}

void configure_dac()
{
	RCC->APB1ENR |= RCC_APB1ENR_DAC1EN; // Connect the DAC to the system clock via the APB1 peripheral clock bus.
	DAC1->CR |= DAC_CR_BOFF1; // Disable the 'buffer' function in the DAC control register.
	DAC1->CR |= DAC_CR_EN1; // Enable the DAC peripheral.
	
	/*
	*	Configure PA.4 to be an analogue output.
	*/
	GPIOA->MODER &= ~(0x300);
}

void configure_leds()
{
		RCC->AHBENR |= RCC_AHBENR_GPIOEEN;	// Enable clock on GPIO port E

		GPIOE->MODER |= 0x55550000; // Set mode of each pin in port E
		GPIOE->OTYPER &= ~(0x00000100); // Set output type for each pin required in Port E
		GPIOE->PUPDR &= ~(0x00000000); // Set Pull up/Pull down resistor configuration for Port E
}

void configure_adc()
{
	delay_ten_microseconds();
			
	// 1. Reset and then enable the Voltage Regulator on the ADC1_CR register:
	// ADC1->CR &= ~(0x30000000);
	ADC1->CR |= 0x20000000;
	ADC1->CR |= 0x10000000;
	
	delay_ten_microseconds();
	
	// 2.1 Calibrate the ADC using the ADC1_CR register:
	ADC1->CR &= ~(0x40000000); // Set the calibration setting as single ended:
	ADC1->CR &= ~(0x80000000); // Start the calibration by writing 1 to ADCAL:
	ADC1->CR |= 0x80000000; // Start the calibration by writing 1 to ADCAL:
	
	// Poll to wait for the calibration to complete.
	while(ADC1->CR & ADC_CR_ADCAL) {}
			
	// 3. Point the peripheral clock to ADC1:
	RCC->CFGR2 |= RCC_CFGR2_ADCPRE12_DIV2;
	RCC->AHBENR |= RCC_AHBENR_ADC12EN;
	ADC1_2_COMMON->CCR |= 0x00010000;
		
	/*
	* Note: PA4 (output of DAC) has been wired to PA0 (input of ADC1).
	*	Now, set the PA0 to analogue mode.
	*/
	GPIOA->MODER |= 0x3;
	
	// 5. Configure the CFGR register to set: 
	ADC1->CFGR &= ~(0x18); // 12-bit resolution.
	//ADC1->CFGR |= 0x10;
	ADC1->CFGR &= ~(0x20); // RH data alignment.
	//ADC1->CFGR &= ~(0x2000); // Not-continious operation:
	
	// 6. Set the multiplexing options. (1 channel, listening to channel 1 (IN1)).
	//ADC1->SQR1 &= ~(0x7CF) // Reset the channel length (L) and first address.
	ADC1->SQR1 |= 0x40;
	
	// 7. Set the sample time of the ADC:
	ADC1->SMPR1 &= ~(0x38);  // mb rm
	ADC1->SMPR1 |= 0x18;
		
	// 8. Enable the ADC:
	ADC1->CR |= 0x1;
}

void delay_ten_microseconds()
{
	int j = 0;
	while(j < 101){
		j++;
	}
	return;
}


// This has a maximum value of 4095 (8 bits).
int dacOutput = 0;
bool increasing = true;

void TIM3_IRQHandler()
{
	if ((TIM3->SR & TIM_SR_UIF) !=0) // Check interrupt source is from the ‘Update’ interrupt flag
	{
		
		if(increasing){
			dacOutput = dacOutput + 32;
		}
		else {
			dacOutput = dacOutput - 32;
		}
		
		if(dacOutput > 4096){
			dacOutput = 4095;
			increasing = false;
		}
		else if(dacOutput < 0){
			dacOutput = 0;
			increasing = true;
		}
		
		DAC1->DHR12R1 = dacOutput;		
	}
	
	TIM3->SR &= ~TIM_SR_UIF; // Reset ‘Update’ interrupt flag in the SR register
}


