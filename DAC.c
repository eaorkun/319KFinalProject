// dac.c
// This software configures DAC output
// Lab 6 requires a minimum of 4 bits for the DAC, but you could have 5 or 6 bits
// Runs on LM4F120 or TM4C123
// Program written by: Stephen Do and Eralp Orkun
// Date Created: 3/6/17 
// Last Modified: 3/30/2020  
// Lab number: 6
// Hardware connections
// Output PortB 0-5

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

// **************DAC_Init*********************
// Initialize 6-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
	//Sets PB0-5 as outputs
	SYSCTL_RCGCGPIO_R |= 0x02;
	volatile int nop;
	nop++;
	nop++; 
	GPIO_PORTB_DIR_R |= 0x3F;
	GPIO_PORTB_DEN_R |= 0x3F;
}

// **************DAC_Out*********************
// output to DAC
// Input: 6-bit data, 0 to 63
// Input=n is converted to n*3.3V/63
// Output: none
void DAC_Out(uint32_t data){
	 uint32_t holder = 0;
	 holder = GPIO_PORTB_DATA_R & ~0x3F; //Bit clear
   GPIO_PORTB_DATA_R = holder | (0x3F & data); //writes to DAC
}
