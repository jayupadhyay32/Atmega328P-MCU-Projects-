/*
 * Lab2a_part1.c
 *
 * Created: 9/17/2016 1:32:09 PM
 * Author : Jay
 */ 

// This program will blink a single LED which is on-board at two different rates.
// Remember that a switch is connected to PD2 and PB7
// On-board LED is connected to PINB5.
// Little more done for fun and experimentation. 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRB |= 1<<DDB5;						// Set the fifth pin of DDRB as output.           
	DDRD = 0b11111111;						// Set all of the bank D pins as output. 
	
    while (1) 
    {
		if(!(PINB & (1<<PINB7)))			// If the button is pressed, blink @ 8Hz
		{
			PORTB |= 1<<PORTB5;				// LED turns on.
			PORTD = 0b111111111;			// Turn on all of PORTD.
			_delay_ms(125/2);				// Wait 125 ms
			PORTB &= 0;						// LED turns off.
			PORTD &= 0;						// Turn off all of PORTD.
			_delay_ms(125/2);				// Wait 125 ms. 
		}
		else
		{
			PORTB |= 1<<PORTB5;				// Turn the LED on.
			PORTD = 0b111111111;			// Turn on all of PORTD
			_delay_ms(250);					// Wait for 250 ms
			PORTB &= 0;						// Turn LED off.
			PORTD &= 0;						// Turn off PORTD
			_delay_ms(250);					// Wait another 250 ms. 
		}
		
	 }
}

