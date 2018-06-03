/*
 * Lab_assignment3B_isr.c
 *
 * COPY of the lab solution.
 * Couldn't figure it out myself, needed to copy and paste and analyze. 
 * .  
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


#define part1A					1  // State before first delay.
#define part1B					2  // State before second delay.
#define part1C					3  // ... before third delay.
#define part1D					4  // same thing as the guy above said.
#define part2A					5  // Starting from this, we will execute the data-write segment.
#define part2B					6  // Still writing!
#define part2C					7  // part 3 of writing.
#define part2D					8  // I'm getting bored of typing.
#define FINISHED			    9  // ... finished.
#define middle					10   // For only command-writes.
#define initTime				1000
#define QueueSIZE 100

uint8_t array[QueueSIZE];
int size = 0;
int index = 0;
int front = 0;

volatile unsigned char curr_state = part1A;
volatile int SWTaskTimer1 = initTime;  // 1 second timer.
volatile int new_req = 0;
volatile int lcd_busy = 0;
volatile int toggle = 0;
volatile int char_counter = 0;
volatile int only_command = 0;
volatile int only_data = 0; 
typedef struct request {
	
	volatile uint8_t data;			// Data Instruction stored here.
	volatile uint8_t command;       // Command instruction stored here. 
		
} request_t; // structure that holds two int values.

request_t lcd_write; // A variable that holds two integers, data and a command.  

// Create a holder for the structure. 
// this will hold a command instruction or data instruction
// This structure will only be emptied once the given instruction is complete.

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
	LCD_CommandWrite_noBlock(0x01);
	InitTimer1();		// Timer will be incremented every 1ms. 
}

// This function will do a command-write first, and finally a data-write. 
void LCD_STATE_MACHINE(void) // ISR calls this every 1ms, finishing each write in 5ms.
{
	switch(curr_state){
	
	case part1A:
	if(new_req == 1)			// If we got a new request, otherwise break this function call. 
	{	
	DATA_PORT = (DATA_PORT & 0xf0) | (lcd_write.command >> 4);
	CTRL_PORT &= ~(1<<RS);
	CTRL_PORT |= 1<<ENABLE;
	curr_state = part1B;
	}
	break;
		
	case part1B:
	CTRL_PORT &= ~(1<<ENABLE);
	curr_state = part1C;
	break;
			
	case part1C:
	DATA_PORT = (DATA_PORT & 0xf0) | (lcd_write.command & 0x0f);
	CTRL_PORT &= ~(1<<RS);
	CTRL_PORT |= 1<<ENABLE;
	
	
	curr_state = part1D;
	break;
		
	case part1D:
	CTRL_PORT &= ~(1<<ENABLE);
	if(only_command == 1)
	curr_state = middle;
	else
	curr_state = part2A;
	break;
	
	case middle:				// Dummy-state for only command writes. Will only be set to this if it's only a command-write.
	lcd_busy = 0;
	curr_state = part1A;	   // Link back to start, we only wanted to do a command-write. 
	new_req = 0;
	only_command = 0;
	break;
	
	case part2A:
	if(new_req){               // If we got a new function call, this will execute, otherwise break this function call. 
	DATA_PORT = (DATA_PORT & 0xf0) | (lcd_write.data >> 4);
	CTRL_PORT |= (1<<RS);
	CTRL_PORT |= (1<<ENABLE);
	curr_state = part2B;
	}
	break;
	
	case part2B:
	CTRL_PORT &= ~(1<<ENABLE);
	curr_state = part2C;
	break;
	
	case part2C:
	DATA_PORT = (DATA_PORT & 0xf0) | (lcd_write.data & 0x0f);
	CTRL_PORT |= (1<<RS);
	CTRL_PORT |= (1<<ENABLE);
	curr_state = part2D;
	break;
	
	case part2D:
	CTRL_PORT &= ~(1<<ENABLE);
	curr_state = FINISHED;
	break;
	
	case FINISHED:
	lcd_busy = 0;
	new_req = 0;         // Current request is over. 
	curr_state = part1A; // Link back to the start. 
	break;
  }
}


void LCD_GoTo_and_Write(int x, int y, char data)
{
	while(lcd_busy);									// Stop incoming calls until the current has finished. 
	lcd_busy = 1; 
	register uint8_t DDRAMAddr;
    
	switch(y)
	{
		case 0: DDRAMAddr = LCD_LINE0_DDRAMADDR+x; break;
		case 1: DDRAMAddr = LCD_LINE1_DDRAMADDR+x; break;
		default: DDRAMAddr = LCD_LINE0_DDRAMADDR+x;
	}
    new_req = 1; 
    lcd_write.command = (1<<LCD_DDRAM | DDRAMAddr);       // Store this in the command-data of the structure. 
    lcd_write.data = data;								  // Store the input data inside the data variable of the structure. 
}

void LCD_CommandWrite_noBlock(uint8_t cmd)				  // For only command-writes. 
{
	while(lcd_busy); // This will lockout any other Command-Writes until first one is finished.
	lcd_busy = 1;
	only_command = 1;
	 
	lcd_write.command = cmd;				// Store the input data inside structure.
	
	new_req = 1;						// We got a new request. 
	
}

void LCD_DataWrite_noBlock(uint8_t data)				// For only data-writes. 
{
	while(lcd_busy);			// Keep running until DataWrite is done, lockout everything else. 
	lcd_busy = 1;
	lcd_write.data = data;     // Store data inside the structure.
	new_req = 1;
	curr_state = part2A;	   // Set the current_state to the start of data-write.
	
	 	
}

ISR(TIMER1_COMPA_vect)					// Since ISR's are automatically called, keep calling our state machine every 1ms. 
{
	if(SWTaskTimer1>0)					// This decrements every 1ms. 
	SWTaskTimer1--;
	
	LCD_STATE_MACHINE();				// This function is executed every 1ms
}

void InitTimer1(void)                   // (8/(16Mhz))(1+1999) = 1ms  (Exactly 1 millisecond)  
{
	OCR1A = 1999;						// 250 ticks.
	TIMSK1 |= (1<<OCIE1A);              // Turn on Timer0 compare match ISR.
	TCCR1B |= (1<<WGM12);               // Enable CTC mode. 
	TCCR1B |= (1<<CS11);                // Set prescaler to 8. 
}

void PrintString(char* c)
{
	for(int i = 0; i<strlen(c); i++)
	LCD_DataWrite_noBlock((uint8_t)c[i]);
	
}


void LCD_REFRESH(void)
{	
	uint8_t lcd_data = (toggle)?'0':'1';             // if toggle is 1, set to 0 else set to 1.
    LCD_GoTo_and_Write(0,0,lcd_data);	             // If 1 print 0, otherwise print 1.
	LCD_GoTo_and_Write(0,1,(uint8_t)(toggle + '0')); // convert into to char, then to an 8 bit integer.
    char_counter++;
	toggle ^= 1;						             // Change toggle value. 
}


int main(void)
{
    init_all();
	sei();
	DDRB = 1<<DDB5;
    while (1) 
    {
	   
	  if(SWTaskTimer1<=0) // Data-write will occur every second.
	  {
		  
		  if(char_counter==16) // print 5 chars then clear test. 
		  {
			  LCD_CommandWrite_noBlock(0x01);
		  }	  
		  PORTB ^= 1<<PORTB5; 
		  LCD_REFRESH();
		  SWTaskTimer1 = initTime;
		  
	  } 
	  
	}
}


