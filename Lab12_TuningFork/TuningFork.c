// TuningFork.c Lab 12
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3, PB3, or PE3.
// There is an output on PA2, PB2, or PE2. The output is 
//   connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include "TExaS.h"
#include "..//tm4c123gh6pm.h"

static int flag, current, last;
//0x4000.4000 
#define PA3                   (*((volatile unsigned long *)0x40004020))
#define PA2                   (*((volatile unsigned long *)0x40004010))

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode

// input from PA3, output from PA2, SysTick interrupts
void Sound_Init(void){ 
	SYSCTL_RCGC2_R |= 0x00000001; 
	GPIO_PORTA_AMSEL_R &= ~0x0C;    
  GPIO_PORTA_PCTL_R &= ~0x0000FF00; 
  GPIO_PORTA_DIR_R |= 0x04;    
	GPIO_PORTA_DIR_R &= ~0x08;   
  GPIO_PORTA_DR8R_R |= 0x04;   
  GPIO_PORTA_AFSEL_R &= ~0x0C; 
	GPIO_PORTA_PDR_R |= 0x08;     
  GPIO_PORTA_DEN_R |= 0x0C;       
  PA2 = 0x00;	
  NVIC_ST_CTRL_R = 0; 
  NVIC_ST_RELOAD_R = 90908;   // reload value for 880Hz => (1/880Hz) / (1/80Mhz)
  NVIC_ST_CURRENT_R = 0;        
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; 
  NVIC_ST_CTRL_R = 0x07;  
}

// called at 880 Hz
void SysTick_Handler(void){
	if(flag)
	{
		PA2 ^= 0x04;
	}
	else
	{
		PA2 = 0x00;
	}
	NVIC_ST_RELOAD_R = 90908;
}

int main(void){// activate grader and set system clock to 80 MHz
  TExaS_Init(SW_PIN_PA3, HEADPHONE_PIN_PA2,ScopeOn); 
  Sound_Init();       
  flag = 0;
  current = 0;
  last = 0;  
  EnableInterrupts();   // enable after all initialization are done
  while(1){
		int i = 999;
		while(i--)
		{
			current = PA3;
			if(current == 0x08 && last == 0x0)
			{
					flag ^= 1;
			}			
			last = current;
		}
  }
}
