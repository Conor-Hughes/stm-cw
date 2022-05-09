/* 
	Pete Hubbard 2019
	Loughborough University
	WSC055 Lab 1
	V.2.0
	
	The following 'c' code presents an outline for you to adapt during the laboratory
	
	*/

#include "stm32f3xx.h"                  // Device header

int i = 0;

void adc_setup(void);
void delay_ten_microseconds(void);
void configure_leds(void);
void configure_timer(void);
void configure_dac(void);

int main(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOEEN;	// Enable clock on GPIO port E
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // Direct clock pulses to timer 3
	
	configure_timer();
	
	configure_leds();
	
	configure_dac();
	
	/*
	*	Enable the interrupt for when the ADC is ready:
	*/
	ADC1->IER = 0x5;
	NVIC_EnableIRQ(ADC1_IRQn);
	
	/*
	*	Configure the ADC:
	*/
	adc_setup();
	
	TIM3->DIER |= TIM_DIER_UIE; // Set DIER register to watch out for an ‘Update’ Interrupt Enable (UIE) – or 0x00000001
	NVIC_EnableIRQ(TIM3_IRQn); // Enable Timer ‘x’ interrupt request in NVIC
			
	while (1){}

}

void TIM3_IRQHandler()
{
	if ((TIM3->SR & TIM_SR_UIF) !=0) // Check interrupt source is from the ‘Update’ interrupt flag
	{
		/*
		DAC->DHR12R1 = i;
		
		// Turn off all previous LEDS:
		GPIOE->BSRRH = (DAC->DHR12R1 - 1) << 8;
		
		// Reset the counter once it reaches the full value.
		if(i > 255) {
			i = 0;
		}
	
		GPIOE->BSRRL = DAC->DHR12R1 << 8;
		i++;*/
	}
	
	TIM3->SR &= ~TIM_SR_UIF; // Reset ‘Update’ interrupt flag in the SR register
}

// Sets the PSC & ARR values for Timer3 so it sends a signal every 1s.
void configure_timer()
{
	TIM3->PSC = 100;
	TIM3->ARR = 7999;
	TIM3->CR1 |= TIM_CR1_CEN; // Enables the timer.
}

void configure_dac(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_DAC1EN; // Connect the DAC to the system clock via the APB1 peripheral clock bus.
	DAC1->CR |= DAC_CR_BOFF1; // Disable the 'buffer' function in the DAC control register.
	DAC1->CR |= DAC_CR_EN1; // Enable the DAC peripheral.
	
	/*
	*	Configure PA.4 to be an analogue output.
	*	Note: This is the pin that the DAC is outputting to.
	*/
	GPIOA->MODER &= ~(0x300);
	
	return;
}

// Set the GPIOE (LED) pin configurations:
void configure_leds()
{
	GPIOE->MODER |= 0x55550000; // Set mode of each pin in port E
	GPIOE->OTYPER &= ~(0x00000100); // Set output type for each pin required in Port E
	GPIOE->PUPDR &= ~(0x00000000); // Set Pull up/Pull down resistor configuration for Port E
}


/*
*	Configures the appropriate registers to enable our ADC.
*/
void adc_setup()
{
	delay_ten_microseconds();
	
	// 1. Reset and then enable the Voltage Regulator on the ADC1_CR register:
	ADC1->CR &= ~(0x30000000);
	ADC1->CR |= 0x10000000;
	
	delay_ten_microseconds();
	
	// 2.1 Calibrate the ADC using the ADC1_CR register:
	// Set the calibration setting as single ended:
	ADC1->CR &= ~(0x40000000);
	// Start the calibration by writing 1 to ADCAL:
	ADC1->CR |= 0x80000000;
	
	
	// 2.2 Wait for the calibration to complete (when ADCAL returns to 0):
	// NOTE: Just delaying temporarily whilst we figure out how to poll the last bit. TODO: Update this.
	delay_ten_microseconds();
	delay_ten_microseconds();
	delay_ten_microseconds();
	delay_ten_microseconds();
	delay_ten_microseconds();
	delay_ten_microseconds();
		
	/*
	*	3. Point the peripheral clock to ADC1:
	*	NOTE: Not 100% if this is correct. It was taken from Pete's lecture slides but may not be specific to this.
	*/
	RCC->CFGR2 |= RCC_CFGR2_ADCPRE12_DIV2;
	RCC->AHBENR |= RCC_AHBENR_ADC12EN;
	ADC1_2_COMMON->CCR |= 0x00010000;
	
	/*
	*	4.1
	* Note: PA4 (output of DAC) has been wired to PA0 (input of ADC1).
	*	Now, set the PA0 to analogue mode.
	*/
	GPIOA->MODER |= 0x3;
	
	// 5. Configure the CFGR register to set: 
	// 8-bit resolution:
	ADC1->CFGR &= ~(0x18); // First reset these bits.
	ADC1->CFGR |= 0x10; // Now set RES to 10.
	// RH data alignment:
	ADC1->CFGR &= ~(0x20);
	// Not-continious operation:
	ADC1->CFGR &= ~(0x2000);
	
	// 6. Set the multiplexing options. (1 channel, listening to channel 1 (IN1)).
	ADC1->SQR1 = 0x40; // Need to double check this value.
	// NOTE: This is hard-resetting the register and could cause an issue.
	
	// 7. Set the sample time of the ADC:
	ADC1->SMPR1 = 0x18;
	
	// 8. Enable the ADC:
	ADC1->CR &= ~(0x1);

}

void delay_ten_microseconds()
{
	int j = 0;
	while(j < 101){
		j++;
	}
	return;
}

void ADC1_IRQHandler()
{
	while(1){
		DAC->DHR12R1 = i;
		
		// Turn off all previous LEDS:
		GPIOE->BSRRH = 3 << 8;
		
		// Reset the counter once it reaches the full value.
		if(i > 255) {
			i = 0;
		}
	
		GPIOE->BSRRL = 3 << 8;
		i++;
	}
}
