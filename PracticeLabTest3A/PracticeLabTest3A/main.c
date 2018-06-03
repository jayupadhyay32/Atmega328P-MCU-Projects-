/*
 * PracticeLabTest3A.c
 *
 * Created: 10/8/2016 11:24:03 PM
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
#define initTime 1000

#define INTEL           1
#define IBM             2
#define NVIDIA          3
#define GOOGLE          4
#define MICROSOFT       5
#define next            6
#define QueueSIZE       100

char current_state = INTEL; // Start at the first intel advertisement. 

uint8_t array[QueueSIZE];
int size = 0;
int index = 0;
int front = 0;

volatile int SWTaskTimer1 = initTime;  // 1 second timer.


void enqueue(uint8_t x)
{
	array[index+front] = x;
	index++;
	size++;
}

uint8_t dequeue()
{
	uint8_t temp = array[front++];
	index--;
	size--;

	return temp;

}

int Size()
{
	return size;
}

int isEmpty()
{
	return size==0;
}

void init_all(void)
{
	DDRB |= (1<<DDB5);  // 5th pin is the outputLED.
	initialize_LCD();
	LCDcursorOFF();
	InitTimer0();		// Timer will be incremented every 1ms.
}

void PracticeLabTestStateMachinePartA(void)
{
	switch(current_state)
	{
		case INTEL:
		PrintString("I am Intel");
		current_state = IBM;
		break;
		
		case IBM:
		PrintString("I am IBM.");
		current_state = NVIDIA;
		break;
		
		case NVIDIA:
		PrintString("I am Nvidia. ");
		current_state = GOOGLE;
		break;
		
		case GOOGLE:
		PrintString("I am Google.");
		current_state = MICROSOFT;
		break; 
	
		case MICROSOFT:
		PrintString("I am Microsoft");
		current_state = next;
		break;
		case next:
		PrintString("I am a nobody");
		current_state = INTEL;
		break;
		
	}
	
	
}


ISR(TIMER0_COMPA_vect)					// Since ISR's are automatically called, keep calling our state machine every 1ms.
{
	if(SWTaskTimer1>0)					// This decrements every 1ms.
	SWTaskTimer1--;	
}

void InitTimer0(void)                   // Timer for 1ms decays. 
{
	OCR0A = 249;
	TIMSK0 |= (1<<OCIE0A);
	TCCR0A |= (1<<WGM01);
	TCCR0B = (1<<CS01) | (1<<CS00);
}

void PrintString(char* c)
{
	for(int i = 0; i<strlen(c); i++)
	LcdDataWrite(c[i]);
	
}

uint8_t count = 0;
int flag = 0;
int main(void)
{
	init_all();
	sei();
	DDRB = 1<<DDB5;
	while (1)
	{
		
		if(SWTaskTimer1<=0) // Data-write will occur every second.
		{	
			
			enqueue(count++);
			if(count==10)
			{
				SWTaskTimer1 = 5000;   // Wait for 5 seconds :)
				LCDGotoXY(0,1);
				while(!isEmpty())
				LcdDataWrite(dequeue()+'0');
				while(SWTaskTimer1 > 0);
				
				count = 0; 
			}
			else
			{
			LcdCommandWrite(0x01);		
			PracticeLabTestStateMachinePartA(); 
			SWTaskTimer1 = initTime;
			}
		}
		
	}
}


