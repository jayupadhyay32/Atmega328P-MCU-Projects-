/*
 * lab_assignment3c.c
 *
 * Created: 9/27/2016 7:27:09 PM
 * Author : Jay
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include <string.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

#define r_buffer_size 50
char r_buffer[r_buffer_size];
int r_index;
volatile char r_ready; 
FILE uart_str = FDEV_SETUP_STREAM(uart_putchar,uart_getchar,_FDEV_SETUP_RW);
char rec[50];
int toggle = 0;
int time_counter = 0;
int waiting = 0;

ISR(USART_RX_vect)
{
	char r_char = UDR0;
	UDR0 = r_char; // Echo char back out of the system that the human user can see it.
	
	if(r_char != '\r') // compare the enter character. 
	{
		if(r_char == 127) // compare backspace character. 
		{
			putchar(' '); // remove the character on the screen.
			putchar('\b'); // backspace.
			--r_index; // erase previously read character in the r_buffer. 
		}
		else
		{
			r_buffer[r_index] = r_char;
			if(r_index < r_buffer_size-1){r_index++;}
				else {r_index=0;}
		}
	}
	else
	{
		putchar('\n');  // generate a new line.
		r_buffer[r_index]=0; // strings are terminated with a 0.
		r_ready = 1;		
		UCSR0B ^= (1<<RXCIE0); // turn off receive interrupt. 
	}
}

void getstr()
{
	r_ready = 0;						// Clear ready flag.
	r_index = 0;						// Clear the buffer.
	UCSR0B |= (1<<RXCIE0);
}  

void printLCD(char* c)
{
	LCDclr();
	for(int i = 0; i<strlen(c); i++)
	LcdDataWrite(c[i]);
	
}

void Task_InterpretReadBuffer(void)			// Toggle from 2hz to 8hz if yes, otherwise no. Also print on LCD the current mode. 
{
	if(!strcmp("yes",r_buffer))			    // If the user would like to toggle. Value will only toggle if there is some value in the buffer and it's a yes.  
	{
		toggle = !toggle;					// The LED mode has been toggled.   0 = 2hz,   1 = 8hz. 	
	}
	else if(!strcmp("no",r_buffer))
	fprintf(stdout,"Fine, no mode will be changed.\n");
	else
	fprintf(stdout,"hmm... weird input? I need a yes or a no!\n");
	
	if(toggle == 0){
	printLCD("2 Hertz Mode!!");
	fprintf(stdout,"Changing to 2Hz mode. \n");
	}
	else if(toggle == 1)
	{ 
	printLCD("8 Hertz Mode!!");
	fprintf(stdout,"Changing to 8Hz mode. \n");
	}
	
	time_counter = 0;					 // Reset the time counter.
}

void led_blink(int a)
{
	if(a==0)
	{
		PORTB ^= 1<<PORTB5;
		_delay_ms(250);
		time_counter += 250;
	}
	else
	{
		PORTB ^= 1<<PORTB5;
		_delay_ms(62.5);
		time_counter += 62.5;
	}
}


int main(void)
{
	DDRB = 1<<DDB5;   // Pin 5 is the output.
	
	uart_init();      // Initialize the UART. 
	initialize_LCD(); // LCD is initialized. 
	stdout=stdin=stderr=&uart_str;
	sei();						   // Global interrupts initialized. 
    getstr();					   // Reset the r_buffer
    fprintf(stdout,"Hello, welcome to Lab Assignment 3B \n");    
	printLCD("2 Hertz Mode!!");
	
    while (1)					   // Main loop 
    {
		led_blink(toggle);
		
		if(time_counter>=10000)	   // If our value is above or equal to 10 seconds.
		{   
			if(!waiting){
			fprintf(stdout,"Would you like to change modes? yes/no\n");
			waiting = 1;		// Set the flag to wait so the print does not pop up again until something is entered. 
			}
		}
		
		if(r_ready == 1 && time_counter>=10000)	   // If there is something in the buffer, aka some input was received.
		{
			getstr();
			Task_InterpretReadBuffer();
			waiting = 0;						 // Wait flag is reset and after 10 seconds, it will be set.
									     // Clear the value in the buffer to avoid auto toggling.
		}
			
    }
}

