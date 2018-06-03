/*
 * lab2a_c.c
 *
 * Created: 9/17/2016 7:27:09 PM
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
#define  r_buffer_size 20


char r_buffer[r_buffer_size];
int r_index;
volatile char r_ready;
int elapsed = 0;
int freq = 2;
int waiting = 0;
int time_to_toggle = 0;

ISR(USART_RX_vect)
{
	char r_char;
	r_char = UDR0; // Echo char back out of the system that the human user can see it.
	
	UDR0 = r_char; 
	
	if(r_char != '\r') // compare the enter character. 
	{
		if(r_char == '\r') // compare backspace character. 
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

void getstr(void)
{
	r_ready = 0;
	r_index = 0;
	UCSR0B |= (1<<RXCIE0);
}

void toggle_led()
{
	PORTB ^= 1<<PORTB5;
	
	if(freq==2)
	{
		_delay_ms(250);
		
	}
	else
	{
		_delay_ms(62.5);
	}
	
	
}

void switch_modes()
{
	if(!strcmp(r_buffer, "yes"))
	{
		if(freq == 2)
		freq = 8;
		else
		freq = 2;
		
	}
	
	elapsed = 0;  // Reset timer.
	waiting = 0;  // Not waiting anymore.
	time_to_toggle = 0;
    getstr();
	
	
	
}

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar,uart_getchar,_FDEV_SETUP_RW);
char rec[50];


int main(void)
{
	uart_init();
	stdout=stdin=stderr=&uart_str;
    fprintf(stdout,"Hello \n");
	DDRB = 1<<DDB5;
	int toggle = 0;
	int count = 0; // This will hold the time elapsed.
	int waiting = 0;
	getstr();
	sei();
	
    while (1)
    {
		toggle_led();
		
		if(freq == 2)
		elapsed += 250;
		else if(freq == 8)
		elapsed += 62.5;
		
		if(elapsed>=5000) // 10 seconds passed.
		{
			
		time_to_toggle = 1;
		if(!waiting){			
		fprintf(stdout,"Would you like to change modes? \n");
		waiting = 1;  // Statement won't be triggered. 
		}
		}
		
		if(r_ready == 1 && time_to_toggle==1)
		{  
			time_to_toggle = 0;
			waiting = 0;
			elapsed = 0; 
			switch_modes();
		}
		
    }
}

