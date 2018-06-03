/*
 * PracticeLabTest_4.c
 *
 * Created: 10/18/2016 6:17:22 PM
 * Author : Jay
 */ 

#define F_CPU 160000000UL 
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "lcd_lib.h"
#define DELAY 16
#define START 1
#define COMPLETIONOFLAP 2
#define FINISH 3
#define SW1_PRESSED  !(PINB & (1<<PINB1))
#define SW2_PRESSED  !(PINB & (1<<PINB7))


volatile char current_state = START;
volatile int toggleLED_ONE = 0;
volatile int toggleLED_TWO = 0; 
volatile int SW_COUNT = 16;
volatile int SWITCH_ONE_PRESSED = 0;
volatile int SWITCH_TWO_PRESSED = 0;
volatile unsigned long current_time = 0;
volatile unsigned long time_interval = 0;
volatile int enabled = 0;
volatile unsigned long stopwatch_time = 0;
volatile int switchtwo = 0;
volatile unsigned long prev = 0;
volatile unsigned long recorded_time = 0;
volatile unsigned long push_event_time = 0;
volatile unsigned long elapsed = 0;
volatile unsigned long best = 0;
volatile unsigned long old_elapsed = 1000000000; 

void startTimer0(){
	
	TCCR0B |= (1<<CS01) | (1<<CS00);
	
}

void stopTimer0()
{
	TCCR0B &= ~((1<<CS01) | (1<<CS00)); 
}

void initAll(){                                         // Use PD3 and PB1 
	
	initialize_LCD();
	
	DDRD |= (1<<DDD1) | (1<<DDD0);                      // Set the last two LED pins as output. 
	DDRB &= ~((1<<DDB1) | (1<<DDB1));
	DDRD &= ~(1<<DDD3);
	
	PCICR |= (1<<PCIE0);                                   // Enable Pin Change interrupt.
	PCMSK0 |= (1<<PCINT1) | (1<<PCINT7);                   // Mask for PB1   -- Enable
	                 
	
	TCCR0A |= 1<<WGM01;
	OCR0A = 249;
	TIMSK0 = 1<<OCIE0A;
	TCCR0B |= (1<<CS11) | (1<<CS10);
	
	TCCR1B |= (1<<WGM12); 
	OCR1A   = 249;
	TIMSK1 |= (1<<OCIE1A);
	
	
	
	sei();									 // Enable global interrupts.		
	
}

ISR(TIMER0_COMPA_vect)
{
	if(SW_COUNT>0)
	--SW_COUNT;
	
}

ISR(TIMER1_COMPA_vect)
{
	current_time++; 
	
}

ISR(PCINT0_vect) // For Button B (PB1)
{
	PCICR &= ~(1<<PCIE0);
	
	enabled = 1;
	
	old_elapsed = elapsed;
	elapsed = current_time - prev;
	
	if(elapsed < old_elapsed)
	best = elapsed;              // Keep the best time.  
	
    push_event_time = current_time;  // We have a button_push, so let's record the time for this event. ( Last Push Event )  
	
}

void startTimer1()
{
	TCCR1B |= (1<<CS11) | (1<<CS10);
	
}

void stopTimer1()
{
	TCCR1B &= ~((1<<CS11) | (1<<CS10));
}

void state_machine()
{
	
	
	switch(current_state)
	{
		case START:
		if(SW1_PRESSED)
		{   LCDclr();
			current_time = 0;
			time_interval = 0;
			startTimer1();
			PORTD |= 1<<PORTD0;
			//push_event_time = current_time;  
			current_state = FINISH;
		}
		break;
		
		case FINISH:
		if(SW1_PRESSED)
		{   LCDclr();
			//push_event_time = current_time;
			stopTimer1();
			taskB();
			PORTD &= ~((1<<PORTD0) | (1<<PORTD1));
			taskD();
			current_state = START;
		}
		if(SW2_PRESSED){
			
			PORTD ^= 1<<PORTD1;
			prev = current_time; 
			taskC();  
		}
		break;
	}
	 enabled = 0;
	 PCICR |= (1<<PCIE0);
}


void taskC()
{
	LCDGotoXY(0,1);
	
	char buff[10];
	sprintf(buff,"%lu ms", elapsed);
	LCDstring((uint8_t*) buff,strlen(buff));
		
}

void taskD()
{
	LCDGotoXY(0,0);
	
	char bufflop[10];
	sprintf(bufflop,"Time: %lu ms", push_event_time);
	LCDstring((uint8_t*) bufflop,strlen(bufflop));
	
	LCDGotoXY(0,1); 
	
	char secondbuff[12];
	sprintf(secondbuff,"Best: %lu ms", best);
	LCDstring((uint8_t*) secondbuff,strlen(secondbuff));
}


void taskB()
{
	time_interval = current_time;    // Record the current time.
	printToLCD();
}

void partA()
{

	
	if(SW1_PRESSED && SWITCH_ONE_PRESSED)
	{
		stopTimer0();
		LCDclr();
		taskB();
		current_time = 0;
		SWITCH_ONE_PRESSED = 0;
		time_interval = 0;
	}
	
	if(SW1_PRESSED)				 // Start of LAP
	{
		PORTD ^= 1<<PORTD0;      // Turn the first LED on.
		startTimer0();    
		SWITCH_ONE_PRESSED = 1;
		current_time = 0;
		
	}
		
	if(SWITCH_ONE_PRESSED && SW2_PRESSED) // COMPLETION OF LAP
	{
		PORTD ^= 1<<PORTD1; // Toggle the second LED.
		SWITCH_TWO_PRESSED = 1;
		switchtwo++;
		//taskC(); 
	}
	if(SW1_PRESSED && SWITCH_TWO_PRESSED) // FINISHED
	{
		PORTD = 0;				             // Turn off both LED's.
		SWITCH_TWO_PRESSED = 0;
		SWITCH_ONE_PRESSED = 0;              // Show the the first LED is on.
		time_interval = current_time;        // Record the current time.
		printToLCD();
		switchtwo = 0; 
	}
	
}

void printToLCD()
{
	char buffer[20];
	LCDclr();
	LCDGotoXY(0,0); // First row
	sprintf(buffer,"%lu ms",current_time);
	LCDstring((uint8_t*) buffer,strlen(buffer));
	
	current_time = 0;
}


int main(void)
{
    initAll();	
    
    
    while (1) 
    {
	   if(SW_COUNT<=0 && enabled)
	   {
		   state_machine();
	   }
		
	}
	
}

