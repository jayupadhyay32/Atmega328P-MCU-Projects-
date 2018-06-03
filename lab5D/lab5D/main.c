/*
 * lab_assignment5D.c
 *
 * Created: 10/26/2016 4:15:41 PM
 * Author : Jay
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/io.h>
#include "lcd_lib.h"
#define Vref 5.00

volatile int Ain, AinLow;
volatile float Voltage;
volatile float Variance;
volatile float temp_c;
volatile float temp_f; 

char VoltageBuffer[10];

void printString(char* c)
{
	for(int i = 0; i<strlen(c); i++)
	LcdDataWrite(c[i]);
}

void initialize_all()
{
	//init the A to D converter
	
	DDRD = 0b11111111;  // All of PORTD is set to output.
	
	TCCR1A |= (1<<WGM11) | (1<<WGM10) | (1<<COM1A1);				// SET PWM by setting values in A.
	TCCR1B |= (1<<WGM13) | (1<<WGM12);				// SET PWM by setting values in B as well.
	TCCR1B |= (1<<CS10);				// Set the prescaler to 1024.
	OCR1A = 15999;									// Set the tick target to 15624.    (16Mhz/(1024*(15624+1)) = 1Hz PWM
	OCR1B = 7813;                                   // 50% duty rate. 15624*.50 = 7813
	
	TIMSK1 |= (1<<OCIE1A) | (1<<OCIE1B);
	
	
	ADMUX |= (1<<MUX1) | (1<<MUX2) | (1<<ADLAR) ;
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	
	ADCSRA |= (1<<ADSC);
	
	sei();
	
	
}
ISR(TIMER1_COMPA_vect)
{
	PORTD = 0xFF;
}

ISR(TIMER1_COMPB_vect)
{
	PORTD = 0x00;		// Keep light off for OCR1A - OCR1B ticks and on for ( 0 to OCR1B ) ticks.
}


void vary_voltage()
{
	Variance = Voltage/(Vref);  // Ratio of darkness. Higher value = More Darkness. 
	
	OCR1B = Variance * OCR1A;     // The duty rate will vary upon 
	
}

void temperature()
{
	temp_c = ((Voltage*1000)-400)/19.5;
	
	temp_f = (temp_c)*(1.8) + 32; 
}

int main(void)
{
    /* Replace with your application code */
	initialize_all();
	initialize_LCD();
	LCDcursorOFF();
	char lcd_buffer[10];
    while (1) 
    {
	//	LCDGotoXY(0,0);
	//	printString("Voltage:");
		
		ADCSRA |= (1<<ADSC);
		// Read from ADCH to get the 8 MSBs of the 10 bit conversion
		Ain = ADCH;
		// Typecast the volatile integer into floating type data, divide by maximum 8-bit value, and
		// multiply by 5V for normalization
		Voltage = (float)Ain/256.00 * 5.00;
		//ADSC is cleared to 0 when a conversion completes. Set ADSC to 1 to begin a conversion.
		
		
		//vary_voltage();   // Use this for the light-sensor. 
		
		
		temperature();      // use this for the temp sensor. 
		
		LCDGotoXY(0,0);
		
		dtostrf(temp_f,4,3,lcd_buffer);		//LCDGotoXY(0,1);
		LCDstring((uint8_t*)(lcd_buffer),strlen(lcd_buffer));
		LCDGotoXY(8,0);
		printString(" F");
		
		dtostrf(temp_c,4,3,lcd_buffer);		LCDGotoXY(0,1);
		LCDstring((uint8_t*)(lcd_buffer),strlen(lcd_buffer));
		
		LCDGotoXY(8,1);
	    printString(" C");
		
		
		
		_delay_ms(100);		
    }
	
	
	
}

