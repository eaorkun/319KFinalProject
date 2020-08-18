// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Eralp Orkun and Stephen Do
// This is the project for the EE319K Lab 10

// Last Modified: Today
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php

// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground
// STR DISPLAY 160 by 128 16-bit color pixels.
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Print.h"
#include "Random.h"
#include "PLL.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
void GameDraw(void);
void buttons_Init(void);
void inputs_In(void);
void startGame(void);
void endOfGame(uint32_t);
void SysTick_Init(void);
void generateRandomSequence(void);
void introSequence(void);
void endOfGameWin(uint32_t score);

int32_t inputFlag =0;
int32_t inputMailbox =0;

void PortF_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x20; // clock to port f
	volatile int nop;
	nop++;
	nop++; 
	
	GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;            //port F init
	GPIO_PORTF_CR_R = GPIO_PORTF_CR_R | 0xFF;
	GPIO_PORTF_DIR_R &= ~0x011; //F3 and F7 as inputs
	GPIO_PORTF_DEN_R |= 0x011; 
	GPIO_PORTF_PUR_R |= 0x11;  //negative logic???
}

int32_t PortF_Input(void){
	uint32_t portFFixed= 0;
	portFFixed= (~GPIO_PORTF_DATA_R)&(0x11);
	return portFFixed;
	
}

typedef struct state stringtext;

struct state {
	char *english;
	char *spanish;
};

stringtext stringselect[5] = {
  {"Your score is:","Tu puntaje es:"},
	{"Switch it!", "Cambialo!"},
	{"Bop it!", "Presionalo!"},
	{"Slide it!", "Deslízalo!"},
	{"You reached the end!", "llegaste al final!"}
};

struct button {
	int32_t x;      // x coordinate
  int32_t y;      // y coordinate
  int32_t w,h;  // width and height
  const unsigned short *imagePressed; // ptr->image
	const unsigned short *imageNotPressed; // ptr->image
	int32_t status; 
};
typedef struct button button_t;

button_t controls[6] = { 
	{2, 50,21, 18, buttonsmallpressed, buttonsmallblue,0},
	{2, 85,21,18, buttonsmallpressed, buttonsmallred,0},
	{2, 125,21,18, buttonsmallpressed, buttonsmallyellow,0},
	{55, 100, 54, 46, buttonbigpressed, buttonbigblue, 0},
	{55, 100, 54, 46, buttonbigpressed, buttonbigred, 0},
	{55, 100, 54, 46, buttonbigpressed, buttonbigyellow, 0}
};

int spanishflag = 0;
int gamestarted = 0;                        // 0-start menu, 1-user inputs, 2-game displays sequence
uint8_t boardlast = 0;
uint8_t boardnow = 0;
uint8_t titlecontrol = 0;
uint8_t displaycontrol = 0;
uint8_t displayturncounter = 0;
uint8_t barDisplayed = 0;

#define sequenceLength 4         //change to adjust lenght of somputer engerate sequence.
uint32_t turnNumber =0;      
uint32_t inputNumber =0;
uint32_t correctSequence[sequenceLength];
int main(void){  //actual game
  DisableInterrupts();
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
	SysTick_Init();
	ADC_Init();
	Timer1_Init(&inputs_In, 80000000/30);
  Output_Init();
  buttons_Init();
	PortF_Init();
	Sound_Init();
	introSequence();
	while(1){
		GameDraw();
	};
}

void SysTick_Init(void){
  NVIC_ST_CTRL_R &= ~0x03; //bic
	NVIC_ST_RELOAD_R = 0; // set off
	NVIC_ST_CURRENT_R = 0; // reset
	NVIC_ST_CTRL_R |= 0x07; //interupt enabled
}

int16_t timePassed = 0;
uint32_t blackWritePosition = 128;    // exposition of bar that depletes timer
void SysTick_Handler(void){  
	static uint8_t extraDelay = 0;
	if (gamestarted == 0){                    //Intro screen - flashes the title
		extraDelay = (extraDelay + 1)%3;
		if (extraDelay == 2){
			if (titlecontrol == 0){
				titlecontrol = 1;
			} else if (titlecontrol == 1){
				titlecontrol = 0;
			}
		}
	} else if (gamestarted == 1){       //User stage - see if too much time has passed between inputs. secondsPassed needs to reset each user input
		  blackWritePosition = blackWritePosition - 5;
		  timePassed++;
			if(timePassed % 5 == 0){
				Sound_Tick();
			}
			if(timePassed == 26){
				endOfGame(turnNumber);
			}
	} else if (gamestarted == 2){              //	// Computer Stage - display input(in incorrectSequence[]), delay, repeat, up to turnNumber, switch over to user stage                                                //gamestarted ==1; //switch to user stage
		extraDelay = (extraDelay + 1)%10;
		blackWritePosition = 128;
		barDisplayed = 0;
		if (extraDelay == 4){
			if (displayturncounter <= turnNumber){
				displaycontrol = 1;
			} else {
				gamestarted = 1;
				displaycontrol = 0;
				displayturncounter = 0;
				extraDelay = 0;
				
			}
		} else if (extraDelay == 8){
			displaycontrol = 0;
			displayturncounter++;
		}
	
	}
}

void generateRandomSequence(void){
	for(uint32_t i =0;i<sequenceLength;i++){                 //generates random sequence
			correctSequence[i] = (Random32()%3)+1;
			correctSequence[i] =  correctSequence[i];              //DEBUG
		}
	}

void buttons_Init(void){                 //button -- PD1,  switch -- PD0
	SYSCTL_RCGCGPIO_R |= 0x08;
	volatile int nop;
	nop++;
	GPIO_PORTD_DIR_R &= ~0x03;
	GPIO_PORTD_DEN_R |= 0x03;
}



void inputs_In(void) {     //check controls and adjusts values in controls array
	static int8_t last =0;
  static int8_t sliderHasBeenUp = 0;
	if (gamestarted == 1){
		int8_t data = (GPIO_PORTD_DATA_R & 0x03);
		int32_t dataSlider = ADC_In();
		if((sliderHasBeenUp == 1)&& (dataSlider < 600)){
			sliderHasBeenUp = 0;
			last = 3;
			data = 0;
		}
		if(((last !=0)&&(data == 0))){ //button released
			blackWritePosition = 128;
			barDisplayed = 0;
			if (last == 1){
				Sound_Switch();
			}else if (last == 2){
				Sound_Button();
			} else if (last == 3){
				Sound_Slider();
			}
			timePassed = 0;                            // set input timer back to 0
			if(last == correctSequence[inputNumber]){                 //compare to random array here
				 if(turnNumber == inputNumber){                          //check if end of turn
					 //play turn finished sound
					 turnNumber++;                                      // take to next turn
					 inputNumber =0;
					 for (int p=0; p<3; p++){
						 controls[p].status = 1; 
					 }					 
					 gamestarted = 2;
				 } else{                                            // update for next user input
					 //play correct input sound
					 inputNumber++;                                     
						// debug
				 }
			} else {                                          //wrong number pressed - end game
				endOfGame(turnNumber);
			}
			if (turnNumber == sequenceLength){                                // check if completed entire seqence, if so end game
					    endOfGameWin(sequenceLength);       //place holder for "You win!, Your score is (sequenceLength)"
			}
			last = data;                                                 //update last
		}else if (last !=0){                                  // wait for button release
			if (dataSlider <= 600){
					controls[2].status = 0;
			}			
			if (dataSlider > 3800){
				sliderHasBeenUp = 1;
			}
			last = data;
		} else {      
        if (dataSlider <= 600){
					controls[2].status = 0;
				}					
				if(data == 0){                                                 // button hasn't been pressed - wait for button press
					controls[0].status = 0;                                        // if not pressed set status to 0 so its drawn as not presed
					controls[1].status = 0; 
			} else if(data == 1){   // update status on buttons in array
					controls[0].status = 1;                                       //update status to draw button as pressed
			} else if (data == 2){
					controls[1].status = 1;                                      //update status to draw button as pressed
			}
			if (dataSlider > 600){
					controls[2].status = 1;                                      // update status to draw slider button as pressed
			}
				last = data;                                                    //update last
			  if (dataSlider > 3800){
					sliderHasBeenUp = 1;
			  }
		}
		
}	else if(gamestarted ==0) {
	if((boardlast != 0x10) && (boardnow == 0x10)){	
		startGame();
		Sound_Highpitch();
		}
	if((boardlast != 0x01) && (boardnow == 0x01)){
		spanishflag = 1;
		startGame();
		Sound_Tick();
		}

	boardlast = boardnow;
	boardnow = PortF_Input();
	}
}

void startGame(void){
	Random_Init(NVIC_ST_CURRENT_R); // MOVE TO WHEN BUTTON IS PRESSED ON START MENU. 
	generateRandomSequence();       // Move with line above, needs to be called after random init. After this instruction also reset NVIC Current. 
	ST7735_FillScreen(0x0000);            // set screen to black
	gamestarted = 2;
}

void endOfGame(uint32_t score){
	// display turnNumber as score in english or spanish
	NVIC_ST_RELOAD_R = 0; // set off
	NVIC_ST_CURRENT_R = 0; // reset
	gamestarted=0;
	ST7735_FillScreen(0x0000);
	ST7735_SetCursor(4,6);
	if(spanishflag==1){
		ST7735_OutString((stringselect[0].spanish));
	} else {
		ST7735_OutString((stringselect[0].english));
	}
	turnNumber = 0;
	inputNumber = 0;
	ST7735_SetCursor(10,7);
	LCD_OutDec(score);
	DisableInterrupts();
	while(1){
		
	}
}

void endOfGameWin(uint32_t score){
	// display turnNumber as score in english or spanish
	NVIC_ST_RELOAD_R = 0; // set off
	NVIC_ST_CURRENT_R = 0; // reset
	gamestarted=0;
	ST7735_FillScreen(0x0000);
	if(spanishflag==1){
		ST7735_SetCursor(2,4);
		ST7735_OutString((stringselect[4].spanish));
	} else {
		ST7735_SetCursor(0,4);
		ST7735_OutString((stringselect[4].english));
	}
	ST7735_SetCursor(4,6);
	if(spanishflag==1){
		ST7735_OutString((stringselect[0].spanish));
	} else {
		ST7735_OutString((stringselect[0].english));
	}
	turnNumber = 0;
	inputNumber = 0;
	ST7735_SetCursor(10,7);
	LCD_OutDec(score);
	DisableInterrupts();
	while(1){
		
	}
}

void introSequence(void){
	ST7735_FillScreen(0x2017);            // set screen to red
	Delay100ms(3);
	ST7735_DrawBitmap(15,29, TitleScreen1, 18, 27);
	Delay100ms(3);
	ST7735_DrawBitmap(35,28, TitleScreen2, 18, 18);
	Delay100ms(3);
	ST7735_DrawBitmap(54,36, TitleScreen3, 19, 26);
	Delay100ms(3);
	ST7735_DrawBitmap(83,28, TitleScreen4, 30, 26);
	Delay100ms(3);
	ST7735_DrawBitmap(51,49, TitleScreen5, 7, 7);
	Delay100ms(1);
	ST7735_DrawBitmap(57,49, TitleScreen6, 6, 8);
	Delay100ms(1);
	ST7735_DrawBitmap(63,49, TitleScreen7, 5, 5);
	Delay100ms(1);
	ST7735_DrawBitmap(72,49, TitleScreen8, 6, 7);
	Delay100ms(1);
	ST7735_DrawBitmap(77,49, TitleScreen9, 3, 7);
	Delay100ms(1);
	ST7735_DrawBitmap(79,49, TitleScreen10, 6, 8);
	Delay100ms(1);
	ST7735_DrawBitmap(84,49, TitleScreen11, 5, 5);
	Delay100ms(1);
	ST7735_DrawBitmap(90,49, TitleScreen12, 5, 5);
	Delay100ms(1);
	ST7735_DrawBitmap(98,49, TitleScreen13, 7, 7);
	Delay100ms(1);
	ST7735_DrawBitmap(105,49, TitleScreen14, 5, 5);
	Delay100ms(1);
	ST7735_DrawBitmap(110,49, TitleScreen15, 9, 5);
	Delay100ms(1);
	ST7735_DrawBitmap(119,49, TitleScreen16, 5, 5);
	Delay100ms(1);
	ST7735_DrawBitmap(25,116, TitleScreen0, 78, 65);
	ST7735_DrawBitmap(8,149, TitleScreen17, 104, 25);
	EnableInterrupts();
	NVIC_ST_RELOAD_R = 0xFFFFFF;
	Sound_TitleScreen();
	ST7735_DrawBitmap(8,149, TitleScreen17, 104, 25);
}


void GameDraw(void) {
	static uint8_t sequencedata = 0;
	static uint8_t soundCurrentlyPlaying = 0;         // stops repeated sound calls
	if((gamestarted ==1) || (gamestarted ==2)){
		if ((gamestarted == 1) && (barDisplayed==0)){
			ST7735_DrawBitmap(0,160, Timerbar, 128, 10);
			barDisplayed = 1;
		} else if((gamestarted == 1) && (barDisplayed==1)) {
			ST7735_DrawBitmap(blackWritePosition,160, blackTimerBar, 5, 10);
		}
		uint8_t i = 0;
		for (i=0; i<3; i++){
			if(controls[i].status == 1){
				ST7735_DrawBitmap(controls[i].x, controls[i].y, controls[i].imagePressed, controls[i].w, controls[i].h);
			} else{
				ST7735_DrawBitmap(controls[i].x, controls[i].y, controls[i].imageNotPressed, controls[i].w, controls[i].h);
			}
		}
		
		if(displaycontrol == 1){
			sequencedata = correctSequence[displayturncounter];
			if(sequencedata == 1){ // display input 1
				ST7735_DrawBitmap(controls[3].x, controls[3].y, controls[3].imageNotPressed, controls[3].w, controls[3].h);
				if (soundCurrentlyPlaying != 1){
					Sound_Switch();
				}
				soundCurrentlyPlaying =1;
				ST7735_SetCursor(5,0);
				if(spanishflag==1){
					ST7735_OutString((stringselect[1].spanish));
				} else {
					ST7735_OutString((stringselect[1].english));
				}				
			} else if (sequencedata == 2){ // display input 2
				ST7735_SetCursor(4,0);
				ST7735_DrawBitmap(controls[4].x, controls[4].y, controls[4].imageNotPressed, controls[4].w, controls[4].h);
				if (soundCurrentlyPlaying != 1){
					Sound_Button();
				}
				soundCurrentlyPlaying =1;
				if(spanishflag==1){
					ST7735_OutString((stringselect[2].spanish));
				} else {
					ST7735_OutString((stringselect[2].english));
				}				
			} else if (sequencedata == 3){ // display input 3
				ST7735_DrawBitmap(controls[5].x, controls[5].y, controls[5].imageNotPressed, controls[5].w, controls[5].h);
				if (soundCurrentlyPlaying != 1){
					Sound_Slider();
				}
				soundCurrentlyPlaying =1;
				ST7735_SetCursor(5,0);
				if(spanishflag==1){
					ST7735_OutString((stringselect[3].spanish));
				} else {
					ST7735_OutString((stringselect[3].english));
				}				
		}
		if(displaycontrol == 0){
			soundCurrentlyPlaying =0;
			ST7735_SetCursor(0,0);
			ST7735_OutString("                  ");
			ST7735_DrawBitmap(controls[4].x, controls[4].y, controls[4].imagePressed, controls[4].w, controls[4].h);
		}
	}
	}	
	if(gamestarted ==0){
		Sound_Loop();
		if(titlecontrol == 0){
			ST7735_DrawBitmap(83,28, TitleScreen4, 30, 26);
		}
		if(titlecontrol == 1){
			ST7735_DrawBitmap(83,28, TitleScreenAlt, 30, 26);
		}
	 }
 
}

void gameSound(void){
	
}

// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){
	uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
} 
