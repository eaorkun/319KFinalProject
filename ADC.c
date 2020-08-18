// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Last Modified: 1/17/2020
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){
    SYSCTL_RCGCADC_R |= 0x01;        //ACTIVATE	
		SYSCTL_RCGCGPIO_R |= 0x08;
		while((SYSCTL_PRGPIO_R&0X08) == 0){};
		GPIO_PORTD_DIR_R &= ~0X04;                                                      // PD2 ANALOG PIN SETUP
		GPIO_PORTD_AFSEL_R |= 0X04;     //ALTERNATE FUNCTION ENABLE
		GPIO_PORTD_DEN_R &= ~0X04;      // DIABLE DIGITAL 
		GPIO_PORTD_AMSEL_R |= 0X04;      // ENABLE ANALOG
		ADC0_SAC_R = 6;		
		volatile int nop;
		nop++;
		nop++;
		ADC0_PC_R = 0X01;               //125k Hz                                                //ADC SETUP
		ADC0_SSPRI_R = 0x0123;          // SELECT SEQ3 -- BY PRIORITY
		ADC0_ACTSS_R &= ~0x0008;        //DISABELE SAMPLE SEQUENCER
		ADC0_EMUX_R &= ~0xF000;          //SOFTWAR TRIGGER
		ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+5;      //CHANNEL 5 -> PD2
		ADC0_SSCTL3_R = 0x0006;
		ADC0_IM_R &= ~0x0008;          // DISABLE INTERRRUPTS
			
		ADC0_ACTSS_R |= 0x0008;          // ENABLE SAMPLE SEQUNCER
	
	
}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
	 //  static uint32_t last;
	   uint32_t result;
     ADC0_PSSI_R = 0X0008;                //enable seq3
	   while ((ADC0_RIS_R&0X08)==0){};      // busy wait
		 result = ADC0_SSFIFO3_R&0XFFF;       // read value
		 ADC0_ISC_R = 0X0008;                 //acknowledge
		 //last = result;
		 return result;
}


