//***********************  main.c  ***********************
// Program written by:
// - Steven Prickett  steven.prickett@gmail.com
//
// Brief desicription of program:
// - Initializes an ESP8266 module to act as a WiFi client
//   and fetch weather data from openweathermap.org
//
//*********************************************************
/* Modified by Mairo Leier
 Oct 06, 2016
 Out of the box: to make this work you must
 Step 1) Set parameters of your AP in lines 59-60 of esp8266.c
 Step 2) Change line 39 with directions in lines 40-42
 Step 3) Run a terminal emulator like Putty or TExasDisplay at
         115200 bits/sec, 8 bit, 1 stop, no flow control
 Step 4) Set line 50 to match baud rate of your ESP8266 (9600 or 115200)
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "tm4c123gh6pm.h"

#include "pll.h"
#include "UART.h"
#include "esp8266.h"
#include "LED.h"
#include "Nokia5110.h"


#define MAXLEN 100
char City[100];
char WindSpeed[100];
char WindDirectionDeg[100];
char *WindDirection = NULL;
char Temperature[100];
char Pressure[100];
char Humidity[100];
char Description[100];
char Sunrise[100];
char Sunset[100];
	
time_t curtime;
time_t a;
time_t c;

time(&curtime);




// prototypes for functions defined in startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
char* deg2dir(int deg);

char Fetch[] = "GET /data/2.5/weather?id=588409&APPID=9ec27af034f5c670fe262fe9683c9fb7&units=metric HTTP/1.1\r\nHost:api.openweathermap.org\r\n\r\n";
// 1) go to http://openweathermap.org/appid#use 
// 2) Register on the Sign up page
// 3) get an API key (APPID) replace the 1234567890abcdef1234567890abcdef with your APPID

void ParseResponse(void){ 
char *pt; int i;
	/* find ticker name in response*/
	pt = strstr(ServerResponseBuffer, "\"name\"");
	i = 0; 
	if( pt != NULL ){
		pt += 9; // skip over "name":"
		while((i<100)&&(*pt)&&(*pt!='\"')){
		City[i] = *pt; // copy into City string
		pt++; i++;    
		}
	}
	City[i] = 0;
	
	pt = strstr(ServerResponseBuffer, "\"speed\"");
	i = 0; 
	if( pt != NULL ){
		pt += 8; // skip over "name":"
		while((i<100)&&(*pt)&&(*pt!=',')){
		WindSpeed[i] = *pt; // copy into City string
		pt++; i++;    
		}
	}
	WindSpeed[i] = 0;
	
	pt = strstr(ServerResponseBuffer, "\"deg\"");
	i = 0; 
	if( pt != NULL ){
		pt += 6; // skip over "name":"
		//printf("%s", pt);
		while((i<100)&&(*pt)&&(*pt!='}')){
		WindDirectionDeg[i] = *pt; // copy into City string
		pt++; i++;  		
		}
	}
	WindDirection = deg2dir(atoi(WindDirectionDeg));
	
	  pt = strstr(ServerResponseBuffer, "\"temp\"");
	  i = 0; 
	if( pt != NULL ){
    pt = pt + 7; // skip over "temp":
    while((i<100)&&(*pt)&&(*pt!=',')){
      Temperature[i] = *pt; // copy into string
      pt++; i++;    
		}
	}
	Temperature[i] = 0;
	
	pt = strstr(ServerResponseBuffer, "\"description\"");
	i = 0; 
	if( pt != NULL ){
		pt = pt + 16; // skip over "description":"
    while((i<100)&&(*pt)&&(*pt!='"')){
      Description[i] = *pt; // copy into weather string
      pt++; i++;    
		}
	}
	Description[i] = 0;
	
	
	pt = strstr(ServerResponseBuffer, "\"humidity\"");
	i = 0; 
	if( pt != NULL ){
		pt = pt + 11; // skip over "description":"
    while((i<100)&&(*pt)&&(*pt!=',')){
      Humidity[i] = *pt; // copy into weather string
      pt++; i++;    
		}
	}
	Humidity[i] = 0;
	
	
	
	pt = strstr(ServerResponseBuffer, "\"pressure\"");
	i = 0; 
	if( pt != NULL ){
		pt = pt + 11; // skip over "description":"
    while((i<100)&&(*pt)&&(*pt!=',')){
      Pressure[i] = *pt; // copy into weather string
      pt++; i++;    
		}
	}
	Pressure[i] = 0;
	
	pt = strstr(ServerResponseBuffer, "\"sunrise\"");
	i = 0; 
	if( pt != NULL ){
		pt = pt + 10; // skip over "description":"
    while((i<100)&&(*pt)&&(*pt!=',')){
      Sunrise[i] = *pt; // copy into weather string
      pt++; i++;    
		}
	}
	Sunrise[i] = 0;
	
	c = atoi(Sunrise);
	
	pt = strstr(ServerResponseBuffer, "\"sunset\"");
	i = 0; 
	if( pt != NULL ){
		pt = pt + 10; // skip over "description":"
    while((i<100)&&(*pt)&&(*pt!='}')){
      Sunset[i] = *pt; // copy into weather string
      pt++; i++;    
		}
	}
	
	Sunset[i] = 0;
	a = atoi(Sunset);
}

// Inputs: Number of msec to delay
// Outputs: None
void delay(unsigned long msec){ 
  unsigned long count;
  while(msec > 0 ) {  // repeat while there are still delay
    count = 16000;    // about 1ms
    while (count > 0) { 
      count--;
    } // This while loop takes approximately 3 cycles
    msec--;
  }
}


int main(void){  
  DisableInterrupts();
  PLL_Init(Bus80MHz);

	// Initialize Nokia5110 LCD
	Nokia5110_Init();             
	Nokia5110_Clear();
	Nokia5110_SetCursor(0, 0);
	Nokia5110_OutString((unsigned char*)"************IoT Weather * Lab num: **    16    **          *************");
	delay(1000);
	Nokia5110_Clear();
	Nokia5110_OutString((unsigned char*)"WIFI:conn...");
	delay(100);
	
	Output_Init();       // UART0 only used for debugging
	printf("\n\r-----------\n\rSystem starting...\n\r");
	ESP8266_Init(115200);      // connect to access point, set up as client
	ESP8266_GetVersionNumber();
	
  while(1){
		ESP8266_GetStatus();
    if(ESP8266_MakeTCPConnection("api.openweathermap.org")){ // open socket in server
			Nokia5110_SetCursor(5, 0);
			Nokia5110_OutString((unsigned char*)"OK     ");
			
      if (ESP8266_SendTCP(Fetch) == 1) {
				Nokia5110_OutString((unsigned char*)"NOK   ");
			} else {
				Nokia5110_OutString((unsigned char*)"OK     ");
			}
				Nokia5110_Clear();
				ParseResponse();
			  Nokia5110_SetCursor(0,2);
				Nokia5110_OutString((unsigned char*)City); 
			  Nokia5110_OutString((unsigned char*)"\n");
				Nokia5110_OutString((unsigned char*)Description);
				delay(10);
			  Nokia5110_Clear();
			  Nokia5110_OutString((unsigned char*)"WindSpeed:\n");
			  Nokia5110_OutString((unsigned char*)WindSpeed); 
			  Nokia5110_OutString((unsigned char*)"m/s \n");
				delay(10);
			  Nokia5110_Clear();
				Nokia5110_OutString((unsigned char*)"WindDirection:\n");
			  Nokia5110_OutString((unsigned char*)WindDirection);
				delay(10);
			  Nokia5110_Clear();
			  Nokia5110_OutString((unsigned char*)"Temp:\n");
			  Nokia5110_OutString((unsigned char*)Temperature); 
			  Nokia5110_OutString((unsigned char*)"C \n");
				delay(10);
			  Nokia5110_Clear();
				Nokia5110_OutString((unsigned char*)"Humidity:\n");
			  Nokia5110_OutString((unsigned char*)Humidity); 
			  Nokia5110_OutString((unsigned char*)"% \n");
				delay(10);
			  Nokia5110_Clear();
				Nokia5110_OutString((unsigned char*)"Pressure:\n");
				Nokia5110_OutString((unsigned char*)Pressure); 
			  Nokia5110_OutString((unsigned char*)"hPa \n");
				delay(10);
			  Nokia5110_Clear();
				Nokia5110_OutString((unsigned char*)"Sunrise:\n");
				delay(5);
			  Nokia5110_Clear();
				Nokia5110_OutString((unsigned char*)ctime(&c));
				delay(10);
			  Nokia5110_Clear();
				Nokia5110_OutString((unsigned char*)"Sunset:\n");
				delay(5);
			  Nokia5110_Clear();
				Nokia5110_OutString((unsigned char*)ctime(&a));
			
			  
			
		}

		ESP8266_CloseTCPConnection();
    while(Board_Input()==0){// wait for touch
    }; 
		
  }
	
}


// transparent mode for testing
void ESP8266SendCommand(char *);
int main2(void){  char data;
  DisableInterrupts();
  PLL_Init(Bus80MHz);
  LED_Init();  
  Output_Init();       // UART0 as a terminal
  printf("\n\r-----------\n\rSystem starting at 115200 baud...\n\r");
//  ESP8266_Init(115200);
  ESP8266_InitUART(115200,true);
  ESP8266_EnableRXInterrupt();
  EnableInterrupts();
  ESP8266SendCommand("AT+RST\r\n");
  data = UART_InChar();
//  ESP8266SendCommand("AT+UART=115200,8,1,0,3\r\n");
//  data = UART_InChar();
//  ESP8266_InitUART(115200,true);
//  data = UART_InChar();
  
  while(1){
// echo data back and forth
    data = UART_InCharNonBlock();
    if(data){
      ESP8266_PrintChar(data);
    }
  }
}

char* deg2dir(int deg)
{
	if (deg>11.25 && deg<33.75){
    return "NNE";
	}else if (deg>33.75 && deg<56.25){
    return "ENE";
	}else if (deg>56.25 && deg<78.75){
    return "E";
	}else if (deg>78.75 && deg<101.25){
    return "ESE";
	}else if (deg>101.25 && deg<123.75){
    return "ESE";
	}else if (deg>123.75 && deg<146.25){
    return "SE";
	}else if (deg>146.25 && deg<168.75){
    return "SSE";
	}else if (deg>168.75 && deg<191.25){
    return "S";
	}else if (deg>191.25 && deg<213.75){
    return "SSW";
	}else if (deg>213.75 && deg<236.25){
    return "SW";
	}else if (deg>236.25 && deg<258.75){
    return "WSW";
	}else if (deg>258.75 && deg<281.25){
    return "W";
	}else if (deg>281.25 && deg<303.75){
    return "WNW";
	}else if (deg>303.75 && deg<326.25){
    return "NW";
	}else if (deg>326.25 && deg<348.75){
    return "NNW";
	}else{
    return "N"; 
  }
}


