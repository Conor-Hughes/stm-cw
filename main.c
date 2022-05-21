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

/*
* The output integer tells the program whether we're currently monitoring a combined
*	output (0), the potentiometer (1) or the encoder(2).
*/
volatile uint8_t output = 0;

void configure_dac();
void configure_timer_3();
void configure_timer_2();
void configure_leds();
void configure_adc();
void delay_ten_microseconds();
void convert_potentiometer_signal();
void set_mode(int bitShift);
void update_dac_output();
void setup_button_interrupt();
void setup_pe6_interrupt();
void clear_counter();

	
int main(void)
{
	
	configure_timer_3(); // The timer used for incrementing the DAC output.
	configure_timer_2(); // The timer used for sending out encoder signals.
	configure_leds();
	configure_dac();
	configure_adc();
	setup_button_interrupt();
	setup_pe6_interrupt();
	
	while (1){

		switch (output)
		{
			
			// This means we want the combined output:
			case 0:
				set_mode(8);
				break;
			
			// This means we want the potentiometer output:
			case 1:
				set_mode(9);
				break;
			
			// This means we want the encoder output:
			case 2:
				set_mode(10);
				break;
		}
		
	}
	
}

/**
*	Sets the appropriate LED on for the output mode that we are on.
*/
void set_mode(int bitShift)
{	
	int outputLeds[3] = {8, 9, 10};
	
	for(int i = 0;i<3;i++){
		
		// Turn off the other outputMode LEDs:
		if(outputLeds[i] != bitShift){
			GPIOE->BSRRH = 1 << outputLeds[i];
		}
		// Turn on the LED of the output we are currently controlling:
		else {
			GPIOE->BSRRL = 1 << outputLeds[i];
		}
	}
}

void clear_counter()
{
	GPIOE->BSRRH = 31 << 11; // Reset the existing counter.
}

void convert_potentiometer_signal()
{
		// Set the ADSTART bit high to start the conversion:
		ADC1->CR |= 0x4;
		
		if(ADC1->ISR & 0x4) {
			// Turn off all previous LEDS:
			GPIOE->BSRRH = (DAC->DHR12R1 - 1) << 8;
			
			GPIOE->BSRRL = ADC1->DR << 11;
		}
}

void configure_timer_3()
{
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // Direct clock pulses to timer 3
	
		// TODO: Production values for 250 triggers per second.
		//TIM3->PSC = 320;
		//TIM3->ARR = 99;
	
		// Testing values approx 1/s
		TIM3->PSC = 100;
		TIM3->ARR = 7999;
	
		TIM3->CR1 |= TIM_CR1_CEN; // Enables the timer.
	
		TIM3->DIER |= TIM_DIER_UIE; // Set DIER register to watch out for an ‘Update’ Interrupt Enable (UIE) – or 0x00000001
		NVIC_EnableIRQ(TIM3_IRQn); // Enable Timer ‘x’ interrupt request in NVIC
}

void configure_timer_2()
{
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Direct clock pulses to Timer 1
	
		// These values are for approx 1/s
		TIM2->PSC = 7999;
		TIM2->ARR = 900;
	
		// TODO: Set to these values for production (226 / s)
		//TIM2->PSC = 353;
		//TIM2->ARR = 99;
		
	
		TIM2->CR1 |= TIM_CR1_CEN; // Enables the timer.
	
		TIM2->DIER |= TIM_DIER_UIE; // Set DIER register to watch out for an ‘Update’ Interrupt Enable (UIE) – or 0x00000001
		NVIC_EnableIRQ(TIM2_IRQn);
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

		GPIOE->MODER |= 0x55554000; // Set mode of each pin in port E
		
		
	
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
		
	
	// 5. Configure the CFGR register to set: 
	ADC1->CFGR &= ~(0x18); // 12-bit resolution.
	ADC1->CFGR |= 0x10; // Set to 8-bit resolution.
	//ADC1->CFGR &= ~(0x20); // RH data alignment.
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


void TIM3_IRQHandler()
{
	if ((TIM3->SR & TIM_SR_UIF) !=0) // Check interrupt source is from the ‘Update’ interrupt flag
	{
			update_dac_output();
		
	}
	
	TIM3->SR &= ~TIM_SR_UIF; // Reset ‘Update’ interrupt flag in the SR register
}

// Saves whether the encoder signal is currently high so it can be inverted.
bool encoderHigh = false;

// We're using PE.7 as the output for this encoder signal. (This has already
// been set as an output in the `configure_leds` function.
void TIM2_IRQHandler()
{
	if ((TIM2->SR & TIM_SR_UIF) !=0) // Check interrupt source is from the ‘Update’ interrupt flag
	{
		if(encoderHigh){
			GPIOE->BSRRH = 1 << 7; // Set encoder signal low.
		}
		else {
			GPIOE->BSRRL = 1 << 7; // Set encoder signal high.
		}
		
		encoderHigh = !encoderHigh; // Flip the position of the encoder signal.
	}
	
	TIM2->SR &= ~TIM_SR_UIF; // Reset ‘Update’ interrupt flag in the SR register
}

// This has a maximum value of 4095 (8 bits).
int dacOutput = 0;
bool increasing = true;

/**
	Update the value of the DAC output to create a triangle wave with a resolution of 256.
*/
void update_dac_output()
{
		if(increasing){
			dacOutput = dacOutput + 1;
		}
		else {
			dacOutput = dacOutput - 1;
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

/**
* Sets up the button interrupt for the board to cycle between the different outputs.
*/
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

/**
*	Is called whenever the onboard button is clicked. Cycles between the different output
*	types.
*/
void EXTI0_IRQHandler(){
	if (EXTI->PR & EXTI_PR_PR0) // check source
	{
		EXTI->PR |= EXTI_PR_PR0; // clear flag
		
		output = output + 1;
		
		if(output == 3){
			output = 0;
		}
		
		clear_counter();
	}
}

void setup_pe6_interrupt()
{
	// Remove masks to enable interrupts from EXTI6:
	EXTI->IMR |= EXTI_IMR_MR6;
	
	// Set EXTI6 & EXTI7 ports to generate interrupt on rising and falling edges:
	EXTI->RTSR |= EXTI_RTSR_TR6;
	EXTI->FTSR |= EXTI_FTSR_TR6;
	
	// Configure multiplexing options to enable PE.6 and PE.7 to generate interrupt EXTIO.
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PE;
	
	NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/**
*	Channel PE7 (the output of the square-wave encoder signal) is outputting to PE6.
*	Here, we listen for rising and falling edges, and set an encoder counter to measure
*	the position of the throttle.
*
*	We receive 113 encoder counts for the 80 degrees of movement for the throttle.
* This means we want to add 0.70796460177 on each encoder signal.
*/
float encoderPositionCount = 0;
int incrementingEncoder = true;

void EXTI9_5_IRQHandler(){
	if (EXTI->PR & EXTI_PR_PR6) // check source
	{
		EXTI->PR |= EXTI_PR_PR6; // clear flag*
				
		if(incrementingEncoder){
			encoderPositionCount += 0.70796460177;
		}
		else {
			encoderPositionCount -= 0.70796460177;
		}
		
		int value = (int)round(encoderPositionCount);
				
		// Only show this if we're currently in output mode = 2.
		if(output == 2){
			GPIOE->BSRRH = 31 << 11; // Reset the existing counter.
			
			// Divide the position measurement to prevent overflows, at the expense of accuracy.
			GPIOE->BSRRL = (value / 4) << 11;
		}
		
		// If the encoder is overflowing the max value of 80, reset it to 80 and start decrementing it.
		if(value > 80){
			encoderPositionCount = 80;
			incrementingEncoder = false;
		}
		else if(value < 0){
			encoderPositionCount = 0;
			incrementingEncoder = true;
		}
		
	}
}
