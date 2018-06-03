/*
 * lab_assignment4C.c
 *
 * Created: 10/12/2016 3:41:49 PM
 * Author : Jay
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#define START  0
#define STOP   1
#define RESET  2
#define HOLD   3 
#define TIME   1
#define button_press !(PIND & (1<<PIND3))
#define NOPUSH 1
#define MAYBE  2
#define PUSHED 3

char push_state = NOPUSH;

volatile int Software_counter = TIME;
volatile unsigned long count = 0;
volatile int enabled = 0;
volatile char current_state = START;

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


void switch_debounce()
{
	_delay_us(600);
	
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
		state_machine();  // Call out state_machine. 
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
	//state_machine();     // Update state every time button is pressed.
	switch_debounce();
	//debounce_flag = 1;
      
}

ISR(TIMER0_COMPA_vect) // Timer interrupt.
{
	
	if(Software_counter>0)
	Software_counter--;
	
	if(enabled) // Button is pressed. This will toggle enable updating the clocker. 
	clocker();
}

void Start_Timer()  // This will start the Timer. 
{
	TCCR0B = (1<<CS01) | (1<<CS00);
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
	
	Start_Timer(); 
}

void clocker() // Call this every 1ms
{
	first++;
	
	if(first==10)
	{
		first = 0;
		toggletwo = 1;
		second++; // Increment the value on the right.
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
	
	sei();   // Enable global interrupts.
	
	LCDGotoXY(0,1);
	printString("Jay Upadhyay");
		
    while (1) 
    {
	//	if(debounce_flag)
	//	{
	//	if(button_press){ // If we actually did press the button. 
	//	debounce_flag = 0;  // Reset debounce flag. 
	//	EIMSK |= 1<<INT1;   // Turn interrupt on again.
	//		}
	//	}
		switch_debounce();
		if(Software_counter<=0 && enabled){ // If we receive a button press. Then the program will enable the printing procedure. 
		screenClock();
		Software_counter = TIME;
		}
	}
}

