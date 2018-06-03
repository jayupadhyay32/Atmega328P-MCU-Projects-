/*
 * lab_assignment4D.c
 *
 * Created: 10/12/2016 3:41:49 PM
 * Author : Jay Upadhyay
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "lcd_lib.h"
#define START  0
#define STOP   1
#define RESET  2
#define HOLD   3 
#define TIME   1
#define button_press !(PIND & (1<<PIND3))
#define NOPUSH 1
#define MAYBE  2
#define PUSHED 3
#define POLLINGDELAY 1  // Delay of 1ms 

char push_state = NOPUSH;
char bufferss[10];

volatile int Software_counter = TIME;
volatile unsigned long count = 0;
volatile int enabled = 0;
volatile char current_state = START;
volatile unsigned long x = 0;
volatile int Polling_counter = POLLINGDELAY;

int first = 0;
int second = 0;
int third = 0;
int fourth = 0;
int fifth = 0;
int sixth = 0;
int seventh = 0;
int eight = 0;
int nine = 0;
int ten = 0;

int debounce_flag = 0;

int togglenine = 0;
int toggleight = 0;
int toggleseven = 0;
int togglesix = 0;
int togglefive = 0;
int togglefour = 0;
int togglethree = 0;
int toggletwo = 0;

char buffer[10];  // Buffer of size 10.
//volatile unsigned long y = 0;

void switch_debounce()
{
	if(debounce_flag){                    // We only want to wait if we obtained the INT1 interrupt. 
	while(Polling_counter>0);             // Wait until Polling counter goes to zero i.e, 1ms has elapsed.
	}
	
	switch(push_state)
	{
		case NOPUSH:
		if(button_press)
		push_state = MAYBE;
		else
		push_state = NOPUSH;
		break;
		
		case MAYBE:
		if(button_press){
		push_state = PUSHED;
		state_machine();  // Call our state_machine. 
		EIMSK |= 1<<INT1; // Enable the interrupt. 
		}
		else
		push_state = NOPUSH;
		break;
		
		case PUSHED:
		if(button_press)
		push_state = PUSHED;
		else
		push_state = MAYBE;
		break;
	}
	
	Polling_counter = POLLINGDELAY; // Reset the Polling counter. 
	debounce_flag = 0;
}


void printString(char* c)
{
	for(int i = 0; i<strlen(c); i++)
	LcdDataWrite(c[i]);
}

void state_machine()
{
	
	switch(current_state)
	{
		case START:
		LCDclr();
		enabled = 1;
		current_state = STOP;
		break;
		
		case STOP:
		enabled = 0; 
		current_state = RESET;
		break;
		
		case RESET:
		resetAll();
		enabled = 0;
		x = 0;
		LCDclr();
		printString("Again?");
		current_state = START;	 
		break; 
		
		case HOLD:
		LCDclr();
		current_state = START;
		break;
		
	}
	
}


ISR(INT1_vect) // Pin external interrupt
{
	EIMSK &= ~(1<<INT1); // Disable interrupts temporarily to filter out any glitches.
	switch_debounce();
	debounce_flag = 1;
      
}

ISR(TIMER1_COMPA_vect) // Timer interrupt.
{
	
	//printToLCD();
	if(Software_counter>0)
	Software_counter--;
	
	//if(enabled) // Button is pressed. This will toggle enable updating the clocker. 
	//clocker();
}

ISR(TIMER0_COMPA_vect) // Use this for polling.
{
	x++;
	if(Polling_counter>0)
	Polling_counter--;
   	
    switch_debounce();  
}

void Start_Timer()  // This will start the Timer for both the TIMER1 and TIMER0. 
{
	TCCR0B = (1<<CS01) | (1<<CS00);  // Start Timer0
	TCCR1B = (1<CS11) | (1<<CS10);   // Start Timer1
}

void Stop_Timer()
{
	TCCR0B = 0;
}

void resetAll()
{
	first = 0;
	second = 0;
	third = 0;
	fourth = 0;
	fifth = 0;
	sixth = 0;
	seventh = 0;
	eight = 0;
	nine = 0;
	ten = 0;
	
	toggleight = 0;
	togglenine = 0;
	toggleseven = 0;
	togglesix = 0;
	togglefive = 0;
	togglefour = 0;
	togglethree = 0;
	toggletwo = 0;
	 
}

void initall()
{
	initialize_LCD();
	
	EICRA |= 1<<ISC11; 
	EIMSK |= 1<<INT1;
	
	OCR0A = 249;
	TIMSK0 |= (1<<OCIE0A);
	TCCR0A |= (1<<WGM01);
	
	OCR1A = 249;
	TIMSK1 |= (1<<OCIE1A);     // Turn on compare. 
	TCCR1B |= 1<<WGM12;        // Enable CTC mode. 
	
	Start_Timer();			   // Enable both of the timers. 
}

void clocker() // Call this every 1ms
{
	first++;
	
	if(first==10)
	{
		first = 0;
		toggletwo = 1;
		second++;          
	}
	if(second==10)
	{
		second = 0;
		togglethree = 1;
		third++;
	}
	if(third==10)
	{
		third = 0;
		togglefour = 1;
		fourth++;
	}
	if(fourth==10)
	{
		fourth = 0;
		togglefive = 1;
		fifth++;
	}
	if(fifth==10)
	{
		fifth = 0;
		togglesix = 1;
		sixth++;
	}
	if(sixth==10)
	{
		sixth = 0;
		toggleseven = 1;
		seventh++;
	}
	if(seventh==10)
	{
		seventh = 0;
		toggleight = 1;
		eight++;
	}
	if(eight==10)
	{
		eight = 0;
		togglenine = 1;
		nine++;
	}
	if(nine==10)
	{
		nine = 0;
		ten++;
	}
	if(ten==10)
	{
		resetAll(); // MAX reached, so let's reset the stopwatch.
	}
}

void printToLCD()
{
	LCDGotoXY(0,1);                          // Put the LCD cursor at 0,0
	sprintf(bufferss, "%lu ms", x);          // Take the value, push it onto the buffer and set the string inside as long + ms	LCDstring(bufferss, strlen(bufferss));     // Print the buffer onto LCD, strlen(buffer) will give length of current string which is also the amount of bytes.
	
}

void screenClock() // This will print a seven digit string starting from (0,0).
{
	LCDGotoXY(0,0);    // Clear everything that previously on the screen.  
	
	if(togglenine)
	LcdDataWrite(nine + '0');
	if(toggleight)
	LcdDataWrite(eight + '0');
	if(toggleseven)
	LcdDataWrite(seventh + '0');
	if(togglesix)
	LcdDataWrite(sixth + '0');
	if(togglefive)
	LcdDataWrite(fifth + '0');
	if(togglefour)
	LcdDataWrite(fourth + '0');
	if(togglethree)
	LcdDataWrite(third + '0');
	if(toggletwo)
	LcdDataWrite(second + '0');
	LcdDataWrite(first + '0');
	
	printString("ms");
	
}

int main(void)
{
    initall();
	
	sei();                                   // Enable global interrupts.
	
	//unsigned long y = 5000;
	//LCDGotoXY(0,0);                          // Put the LCD cursor at 0,0
	//sprintf(bufferss, "%lu ms", ++y);          // Take the value, push it onto the buffer and set the string inside as long + ms 	//LCDstring(bufferss, strlen(bufferss));     // Print the buffer onto LCD, strlen(buffer) will give length of current string which is also the amount of bytes. 
	
	LCDGotoXY(0,1);
	printString("Jay Upadhyay");
	
	
	
    while (1) 
    {
		if(Software_counter<=0 && enabled){ // Keep printing on the LCD.
	    //clocker(); 
		//screenClock();
		printToLCD();
		Software_counter = TIME;
		}
	}
}

