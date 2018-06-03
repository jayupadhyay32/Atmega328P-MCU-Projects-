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
#define FINISHED			    5
#define initTime				1000

volatile unsigned char curr_state = part1A;
volatile int SWTaskTimer1 = initTime;  // 1 second timer.
volatile int new_req = 0;
volatile int lcd_busy = 0;
volatile int toggle = 0;
volatile int char_counter = 0;


typedef struct request {
	
	volatile uint8_t data;			// Instruction stored here.
	volatile uint8_t signal;			// Command Type or Data Type stored here. 
	
} request_t;

request_t lcd_write; 

// Create a holder for the structure. 
// this will hold a command instruction or data instruction
// This structure will only be emptied once the given instruction is complete.  
 
void init_all(void)
{
	DDRB |= (1<<DDB5);  // 5th pin is the outputLED.
	initialize_LCD();
	LCDcursorOFF();
	LCD_CommandWrite_noBlock(0x01);
	InitTimer0();		// Timer will be incremented every 1ms. 
}

void LCD_STATE_MACHINE(void) // ISR calls this every 1ms, finishing each write in 5ms.
{
	switch(curr_state){
	
	case part1A:
	if(new_req == 1)			// If we got a new request.
	{
	
	DATA_PORT = (DATA_PORT & 0xf0) | (lcd_write.data>>4);
	
	if(lcd_write.signal)   // if command signal write 
	CTRL_PORT &= ~(1<<RS); // else data signal write
	else
	CTRL_PORT |= (1<<RS);
	
	CTRL_PORT |= (1<<ENABLE); 
	curr_state = part1B;
	}
	break;
		
	case part1B:
	CTRL_PORT &= ~(1<<ENABLE);
	curr_state = part1C;
	break;
			
	case part1C:
	DATA_PORT = (DATA_PORT & 0xf0) | (lcd_write.data & 0x0f);
	if(lcd_write.signal)
	CTRL_PORT &= ~(1<<RS); // if command
	else
	CTRL_PORT |= (1<<RS); // else data 
	CTRL_PORT |= (1<<ENABLE);
	curr_state = part1D;
	break;
	
	case part1D:
	CTRL_PORT &= ~(1<<ENABLE);
	curr_state = FINISHED;
	break;
	
	case FINISHED:
	lcd_busy = 0;
	new_req = 0;  // Current request is over. 
	curr_state = part1A; // Link back to the start. 
	break;
  }
}

void LCD_CommandWrite_noBlock(uint8_t cmd)
{
	while(lcd_busy); // This will lockout any other Command-Writes until first one is finished.
	//lcd_busy = 1;   // We will not initiate another write if the current is not finished.
	lcd_busy = 1;
	lcd_write.data = cmd;				// Store the input data inside structure.
	lcd_write.signal = 1;				// Let the structure know that it's a "command" data
	new_req = 1;						// We got a new request. 
	
	
}

void LCD_DataWrite_noBlock(uint8_t data)
{
	while(lcd_busy);			// Keep running until DataWrite is done, lockout everything else. 
	//lcd_busy = 1;			    // Do not initiate another write if current is still unfinished.
	lcd_busy = 1;
	lcd_write.data = data;		// Store data inside the structure.
	lcd_write.signal = 0;		// Type is data.
	new_req = 1;
	
}

ISR(TIMER0_COMPA_vect)					// Since ISR's are automatically called, keep calling our state machine every 1ms. 
{
	if(SWTaskTimer1>0)					// This decrements every 1ms. 
	SWTaskTimer1--;
	
	LCD_STATE_MACHINE();				// This function is executed every 1ms
}

void InitTimer0(void)
{
	OCR0A = 249;
	TIMSK0 |= (1<<OCIE0A);
	TCCR0A |= (1<<WGM01);
	TCCR0B = (1<<CS01) | (1<<CS00); 
}



void LCD_REFRESH(void)
{	
	uint8_t lcd_data = (toggle)?'0':'1'; // if toggle is 1, set to 0 else set to 1.
	LCD_DataWrite_noBlock(lcd_data);	 // If 1 print 0, otherwise print 1.
    char_counter++;
	toggle ^= 1;						 // Change toggle value. 
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
			  char_counter = 0;
		  }
		  PORTB ^= 1<<PORTB5; 
		  LCD_REFRESH();
		  SWTaskTimer1 = initTime;
	  } 
	  
	}
}


