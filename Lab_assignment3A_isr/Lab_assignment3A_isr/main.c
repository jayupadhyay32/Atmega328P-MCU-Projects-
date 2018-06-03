/*
 * Lab_assignment3A_isr.c
 *
 * LED toggling between 2hz and 8hz depending on the button press. WITH TIMERS.
 * 
 * Implemented with the non-debounced switch along with addition of only one button press.  
 * Created: 9/28/2016 3:56:52 PM
 * Author : Jay   
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/pgmspace.h> 
#include "lcd_lib.h"

#define button_pressed          !(PINB & (1<<PINB2))
#define PUSHED                  1
#define NOPUSH                  2
#define MAYBE                   3
#define TWO_HERTZ				250000
#define EIGHT_HERTZ				62500

unsigned char LastState = NOPUSH;
unsigned char PUSHSTATE = NOPUSH;
int mode;
int two_hertz_time = 250;
int eight_hertz_time = 62; 
volatile int SWTaskTimer = 250;
volatile int SWTaskTimer1 = 125;
 
 
void init_all()
{
	mode = 2;			// Set the mode to 2Hz
	DDRB |= (1<<DDB5);  // 5th pin is the outputLED.
	initialize_LCD();
	LCDcursorOFF();
	LCDclr();
}

ISR(TIMER0_COMPA_vect)
{
	if(SWTaskTimer>0)
	SWTaskTimer--;
}

void InitTimer0(void)
{
	TCCR0A |= (1<<WGM01);  // Clear on compare A.
	OCR0A = 249;		   // Set the number of ticks for A.
	TIMSK0 = 1<<OCIE0A;	   // Enable Timer 0 Compare A ISR.
	TCCR0B = 3;			   // Set the Prescalar and Timer 0 starts. 
}

ISR(TIMER1_COMPA_vect)
{
	if(SWTaskTimer1>0)
	SWTaskTimer1--;
}

void InitTimer1(void)
{
	TCCR0B |= (1<<WGM01);
	OCR0B = 249;
	TIMSK1 = 1<<OCIE0A;
	TCCR0A = 3; 
}

void task1()
{
	if(mode==2)
	{
		mode = 8;		// Set to 8hz mode.
		printString("Zheng-field ON");
		
	}
	else
	{
	    mode = 2;	   // Set to 2hz mode.
		printString("Zheng-field OFF");
		 
	}
	
}

void no_debounce(void)  
{
	if(button_pressed && LastState == NOPUSH)   
	{
									    
		LastState = PUSHED;
	}
	else  
	{
		
		if(!button_pressed)
		{                     
		LastState = NOPUSH;
		}
	}
}



void with_debounce(void)  // De-bounce implementation
{
	switch(PUSHSTATE)
	{
		case NOPUSH:
		if(button_pressed)
		{ 
		LastState = PUSHSTATE;	
		PUSHSTATE = MAYBE;
		}
		else
		{
	    LastState = PUSHSTATE; 
		PUSHSTATE = NOPUSH;
		}
		break;
	   
	    case MAYBE:
		if(button_pressed && LastState == NOPUSH) // Keep the last state to No-Pushed (We will only register a push if its done serially)
		{
			LastState = PUSHSTATE;
			PUSHSTATE = PUSHED;
												// You're task goes here //
			task1(); 
		}
		else
		{
			LastState = PUSHSTATE;
			PUSHSTATE = NOPUSH;
		}
		break;
		
		case PUSHED:
		if(button_pressed) {
			LastState = PUSHSTATE;
			PUSHSTATE = PUSHED;
		}
		else{
			LastState = PUSHSTATE; 
			PUSHSTATE = MAYBE;
		}
		break; 
	}
	
}

void mydelay(int x)
{
	for(int i = 0; i<x; i++)
	_delay_ms(1);
}

void LED_BLINK(int x)
{
	
	if(x==2)
	x = 250;
	else
	x = 62;
	PORTB ^= 1<<PORTB5;
	
	SWTaskTimer = x; 
	
}

void printString(char* c)
{
	LCDclr();
	for(int i = 0; i<strlen(c); i++)
	LcdDataWrite(c[i]);
}


int main(void)
{
    init_all();
	InitTimer0();
	sei();
	printString("Jay Upadhyay"); 
    while (1) 
    {
	  with_debounce(); // Check for button press.
	  
	   
	  if(SWTaskTimer == 0)
	  {	
	    LED_BLINK(mode);
	  }
	  
	  
	}
}


