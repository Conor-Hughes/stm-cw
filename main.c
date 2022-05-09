/* 
	Pete Hubbard 2019
	Loughborough University
	WSC055 Lab 1
	V.2.0
	
	The following 'c' code presents an outline for you to adapt during the laboratory
	
	*/

#include "stm32f3xx.h"                  // Device header


void delay(int a); // prototype for delay function

int main(void)
{
	// Enable clock on GPIO port E
	RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
	
	// GPIOE is a structure defined in stm32f303xc.h file
	// Define settings for each output pin using GPIOE structure
	GPIOE->MODER |= 0x55550000; // Set mode of each pin in port E
	GPIOE->OTYPER &= ~(0x00000100); // Set output type for each pin required in Port E
	GPIOE->PUPDR &= ~(0x00000000); // Set Pull up/Pull down resistor configuration for Port E
		
	int index = 0;
	
	while (1)
  {
		// Reset the counter once it reaches the full value.
		if(index > 255) {
			index = 0;
		}
		
		GPIOE->BSRRL = index << 8;
		delay(1200000);
		GPIOE->BSRRH = index << 8;
		delay(1200000);
		index++;
	}

}

// Delay function to occupy processor
void delay (int a)
{
    volatile int i,j;

    for (i=0 ; i < a ; i++)
    {
        j++;
    }

    return;
}
