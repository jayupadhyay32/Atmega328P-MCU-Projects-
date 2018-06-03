/*
 * lab2a_2.c
 *
 * Created: 9/17/2016 3:56:01 PM
 * Author : Jay
 */ 
#define switch1_pressed							!(PINB & (1<<PINB7))
#define switch2_pressed							!(PINB & (1<<PINB2))
#define F_CPU 16000000UL
#include <avr/io.h>
#include<util/delay.h>

int main(void)
{
    DDRD = 0b11111111;       // All led's for PORTD set as output.
	DDRB = 0b01111011;       // Pin 2 and 7 configured as inputs for bank B
	
							 // initial position of the blinking LED
	char c = 0b00000001;     // c will hold current position.  
    
	while (1) 
    {
		
		if((switch1_pressed))
		{
			PORTD = c;
			_delay_ms(62.5);
			PORTD = 0;
			_delay_ms(62.5);
			
						
		}
		else if((switch2_pressed))
		{
			_delay_ms(100); // Small delay for led transition.
			c = c<<1;
			PORTD = c;	// Shift left 
			_delay_ms(100);
			if(c==0)
			c = 0b00000001;	// Reset the port.
		}
		else
		{
			PORTD = c;
			_delay_ms(250);
			PORTD = 0;
			_delay_ms(250);
		}
    }
}

