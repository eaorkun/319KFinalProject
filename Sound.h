// Sound.h
// Runs on TM4C123 or LM4F120
// Prototypes for basic functions to play sounds from the
// original Space Invaders.
// Jonathan Valvano
// 1/17/2020

#include <stdint.h>
void Sound_Init(void);
void Sound_Play(const uint8_t *pt, uint32_t count);
void Sound_Loop(void);
void Sound_TitleScreen(void);
void Sound_Highpitch(void);
void Sound_Tick(void);
void Sound_Switch(void);
void Sound_Button(void);
void Sound_Slider(void);
