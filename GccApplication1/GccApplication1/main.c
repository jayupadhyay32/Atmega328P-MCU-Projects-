/*
 * GccApplication1.c
 *
 * Created: 9/15/2016 11:42:15 PM
 * Author : Jay
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>



int main(void)
{
	DDRD = 0b11111111;
	DDRB |= 1<<DDB5;
	char led = 0b00000001;
    /* Replace with your application code */
    while (1) 
    {
          
      if(!(PINB & (1<<PINB7)))
      {
		PORTD = led;
        _delay_ms(125);
		PORTD = 0;
		_delay_ms(125);
      }
      else if (!(PINB & (1<<PINB2)))
      {
		led = led<<1;
		PORTD = led;                 // Shift left by one.
		_delay_ms(500);				 // Small time given for button press.
		if(led==0)
		{
		led = 0b00000001;			// Reset led if shifted to zero. 
		}
 
	  }
	  else
      {
		PORTD = led;
		_delay_ms(500);
		PORTD = 0;
		_delay_ms(500); 
      }
    
    }
}
