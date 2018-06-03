/*
 * Lab_assignment3c.c
 *
 * LED togglging between 2hz and 8hz depending on the button press. WITH TIMERS.
 * 
 * Implemented with the non-debounced switch along with addition of only one button press.  
 * Created: 9/28/2016 3:56:52 PM
 * Author : Jay   
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include<util/delay.h>
#include <string.h>
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
 char lcd_buffer[17];
void init_all()
{
	initialize_LCD();
	LCDclr();
	LCDcursorOFF(); 
	mode = 2;			// Set the mode to 2Hz
	DDRB |= (1<<DDB5);  // 5th pin is the outputLED.
	printString("2 Hertz Mode");
	LCDGotoXY(0,1);
	printString("2Hz = 250ms");
}

void task1()
{
	if(mode==2)
	{
		mode = 8;		// Set to 8hz mode.
		LCDclr(); 
		printString("8 Hertz Mode");
		LCDGotoXY(0,1);
		printString("8hz = 62.5ms");
		
	}
	else
	{
	    mode = 2;	   // Set to 2hz mode.
		LCDclr();
		printString("2 Hertz Mode");
		LCDGotoXY(0,1);
		printString("2Hz = 250ms");
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
	with_debounce();			// Check if button was pressed before we delay.
	mydelay(x);
	with_debounce();			// Check if the button was pressed right after the delay. 
}

void printString(char* c)
{
	for(int i = 0; i<strlen(c); i++)
	LcdDataWrite(c[i]);
}



int main(void)
{
    init_all();
    while (1) 
    {
	  LED_BLINK(mode);  // This will launch the LED-blinking procedure with the given mode selected. 
	}
}

