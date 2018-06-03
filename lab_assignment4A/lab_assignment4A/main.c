/*
 * lab_assignment4A.c
 *
 * Created: 10/11/2016 1:16:49 AM
 * Author : Jay
 */ 

#define  F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <stdlib.h>
#include "uart.h"
#define  one_second 1000 
#define  max_time   5000
#define  button_press  !(PINB & (1<<PINB2))
#define QueueSIZE 100

int array[QueueSIZE];
int size = 0;
int ind = -1;
int average = 0;
int reps = 0;

void Push(int a)
{
	array[++ind] = a;
	
	size++;
}

int Pop()
{
	int temp = array[ind];
	size--;
	ind--;
	return temp; 
}

int isEmpty()
{
	return size==0; 
}

int rando;
int delay_time;
int SW_counter = max_time;
int start = 0;
int difference; 
int start_flag = 0;  

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar,uart_getchar,_FDEV_SETUP_RW);

void initializeAll()
{
	uart_init();
	initialize_LCD();
	stdout = stdin = stderr = &uart_str;
	fprintf(stdout, "UART initialized. \n");
	rando = 2 + (rand() % 5);                      // Generate a number between 2 and 5 seconds. 
	delay_time = rando*one_second;				  // Create a first random time interval.
	DDRB |= 1<<DDB5;							  // Set PB5 as output. 
	DDRB &= ~(1<<DDB2);							  // Something with the input pin. 
	
	OCR0A = 249;								  // 250 time ticks.
	TIMSK0 |= (1<<OCIE0A);						  // Compare match mode enabled.
	TCCR0A |= (1<<WGM01);                         // Clear when compared enabled. * CTC MODE *
	
	EICRA |= (1<<ISC11);						  // Set INT0 for falling edge detection.
	EIMSK |= (1<<INT1);							  // Enable the interrupt for INT0.
	
	sei();										  // Global interrupts are enabled.  					
	 
}

void start_timer()								  // Setting the pre-scaler starts the timer. 
{
	TCCR0B = (1<<CS01) | (1<<CS00);
}

void stop_timer()
{
	reps++;
	PORTB = 0;
	difference = start;							   // Store the amount of milliseconds inside difference.
	start = 0;									   // Reset the start counter.
	fprintf(stdout, "Your reaction time is: %d ms\n",difference);
	average += difference;
	average = average/reps; 
	start_flag = 0;								   // Turn off the flag. 
	LCDclr();
	printInteger(difference);
	prinstring(" ms");
	LCDGotoXY(0,1);
	prinstring("Average: ");
	printInteger(average);  
	TCCR0B = 0;									   // Clear the pre-scaler, this will turn off the clock.
	  
}

ISR(INT1_vect)									   // This interrupt will occur during a falling edge (Value of 0 at input due to button press). 
{							
	
	PORTB &= ~(1<<PORTB5);						   // Trigger falling edge. LED turns off here.  
	
}


ISR(TIMER0_COMPA_vect)							  // Set this ISR so that it is called every 1ms 
{
	if(start_flag==1)
	start++;								      // Start will increase by 1 every millisecond. 
	
	if(SW_counter>0)
	SW_counter--; 
}

void prinstring(char* c)
{
	for(int i = 0; i<strlen(c); i++)
	LcdDataWrite(c[i]);
}

void printInteger(int x)  // This method will print a given integer onto the LCD. 
{
	while(x!=0)
	{
		Push(x%10);      // Push integer onto stack from left to right.
		x = x/10;		 // Keep doing this until x goes to zero.  
	}
	
	while(!isEmpty())
	{
		LcdDataWrite(Pop()+'0');      // Print the value onto the LCD! 
	}
}


void mydelay(int a)
{
	for(int i = 0; i<=a; i++)
	_delay_ms(1); 
}

int main(void)
{
	initializeAll();
	
	prinstring("Press button");
	
    while (1) 
    { 
	    //start = uart_getchar(&uart_str);
		 							  												   
		fprintf(stdout, "Ready......?\n");        // Print on the UART that we are asking for a button press in a random amount of seconds.
		
		LCDclr();
		prinstring("Ready????");
		int x = rando * 1000;
		mydelay(x);			                  // Wait a random amount of seconds. 
		LCDclr();
		fprintf(stdout, "GO! \n");				  // Print on UART for the input.
		prinstring("GO!!");
		start_flag = 1;
		start_timer(); 		
		while(!button_press){
							                      // Begin the timer, this will start counting until a pin interrupt occurs. 
		PORTB |= 1<<PORTB5;						  // Turn an LED on for PORTB and trigger a rising edge.
		}
		
		stop_timer();
		
		fprintf(stdout, "Press button to try again to try Again! \n");
		while(!button_press);
		
    }
}

