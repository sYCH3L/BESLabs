// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3

// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0

// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)

// "walk" light connected to PF3 (built-in green LED) 
// "don't walk" light connected to PF1 (built-in red LED)



// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****


#define goW   	0
#define waitW 	1
#define goS   	2
#define waitS 	3
#define walkN		4
#define waitAll	5

struct State {
  unsigned long Light; 
	unsigned long Walk;	
  unsigned long Time;			    
  unsigned long Next[8];	
}; 
typedef const struct State STyp;

unsigned long cState;
unsigned long Input; 

#define	flash1	6 //flash1
#define flash2	7 // flash2
#define flash3	8 // flash12

STyp FSM[9]={
 {0x0C, 0x02, 10,{goW,goW,waitW,waitW,waitW,waitW,waitW,waitW}},
 {0x14, 0x02, 10,{waitAll,waitAll,waitAll,waitAll,waitAll,waitAll,waitAll,goS}},
 {0x21, 0x02, 10,{goS,waitS,goS,waitS,waitS,waitS,waitS,waitS}},
 {0x22, 0x02, 10,{waitAll,waitAll,waitAll,waitAll,waitAll,waitAll,waitAll,walkN}},
 {0x24, 0x08, 10,{walkN,flash1,flash1,flash1,walkN,flash1,flash1,waitAll}},
 {0x24, 0x02, 10,{waitAll,goW,goS,goW,walkN,walkN,walkN,flash1}},
 {0x24, 0x00, 10,{flash2,flash2,flash2,flash2,flash2,flash2,flash2,flash2}},
 {0x24, 0x02, 10,{flash3,flash3,flash3,flash3,flash3,flash3,flash3,flash3}},
 {0x24, 0x00, 10,{waitAll,waitAll,waitAll,waitAll,waitAll,waitAll,waitAll,goW}}
};		




// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// ***** 3. Subroutines Section *****
void SysTick_Init(void);
void SysTick_Wait(unsigned long delay);
void SysTick_Wait10ms(unsigned long delay);
void PortB_Init(void);
void PortE_Init(void);
void PortF_Init(void);

int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
	SysTick_Init();
  PortF_Init();
	PortB_Init();
	PortE_Init();
	
  EnableInterrupts();
  while(1){
		
		GPIO_PORTB_DATA_R = FSM[cState].Light;
		GPIO_PORTF_DATA_R = FSM[cState].Walk;
    SysTick_Wait10ms(FSM[cState].Time);
		Input = GPIO_PORTE_DATA_R&0x07;
		cState = FSM[cState].Next[Input];  
		
  }
 }

void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
  NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock
}
// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(unsigned long delay){
  NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}
// 10000us equals 10ms
void SysTick_Wait10ms(unsigned long delay){
  unsigned long i;
  for(i=0; i<delay; i++){
    SysTick_Wait(800000);  // wait 10ms
  }
}

void PortB_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x02;          // activate Port B
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // no need to unlock
  GPIO_PORTB_AMSEL_R &= ~0x3F;     // disable analog functionality on PB5-0
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF;// configure PB5-0 as GPIO
  GPIO_PORTB_DIR_R |= 0x3F;        // make PB5-0 out
  GPIO_PORTB_AFSEL_R &= ~0x3F;     // disable alt funct on PB5-0
  GPIO_PORTB_DR8R_R |= 0x3F;       // enable 8 mA drive on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;        // enable digital I/O on PB5-0
}

void PortE_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x10;          // activate Port E
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // no need to unlock
	GPIO_PORTE_AMSEL_R &= ~0x07;     // disable analog function on PE2-0
  GPIO_PORTE_PCTL_R &= ~0x00000FFF;// configure PE2-0 as GPIO
  GPIO_PORTE_DIR_R &= ~0x07;       // make PE2-0 in
  GPIO_PORTE_AFSEL_R &= ~0x07;     // disable alt funct on PE2-0
  GPIO_PORTE_DEN_R |= 0x07;        // enable digital I/O on PE2-0
}


void PortF_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x20;          // activate Port F
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // no need to unlock
	GPIO_PORTF_LOCK_R = 0x4C4F434B;  // unlock PortF PF0  
  GPIO_PORTF_CR_R |= 0x1F;         // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R &= 0x00;      // disable analog function
  GPIO_PORTF_PCTL_R &= 0x00000000; // GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R &= ~0x11;       // PF4,PF0 input, PF3,PF2,PF1 output
	GPIO_PORTF_DIR_R |= 0xE;
  GPIO_PORTF_AFSEL_R &= 0x00;      // no alternate function
  GPIO_PORTF_PUR_R |= 0x11;        // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R |= 0x1F;        // enable digital pins PF4-PF0        
}
