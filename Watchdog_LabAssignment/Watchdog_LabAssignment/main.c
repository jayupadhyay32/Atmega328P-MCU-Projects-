/*
 * Watchdog_LabAssignment.c
 *
 * Created: 11/15/2016 12:08:00 AM
 * Author : Jay
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lcd_lib.h"
#include "uart.h"
#include <avr/eeprom.h>
#include <avr/wdt.h>
// EEPROM write addresses
#define eeprom_true 0
#define eeprom_data 1


// Global values
int current_time; 
int counter = 0; 
volatile unsigned int Ain;
volatile float Voltage;
char buffer[17];			  

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

void printString(char* c)
{
	for(int i = 0; i<strlen(c); i++)
	LcdDataWrite(c[i]);
}


void initialize_all()
{
	initialize_LCD();
	LCDcursorOFF();
	LCDclr();
	
	// For ADC conversion.
	ADMUX |= (1<<MUX2) | (1<<MUX1);
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	ADCSRA |= (1<<ADEN);
	
	// For Watch dog. 
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	WDTCSR = (1<<WDIE) | (1<<WDE) | (1<<WDP3);
	
	uart_init();
	stdout = stdin = stderr = &uart_str;
}

void previous_state_EEPROM()
{
	if(eeprom_read_byte((uint8_t*)0) == 'T')					// 0 can be used to read a byte from the EEPROM. 
	{
		counter = eeprom_read_dword((uint32_t*)1);              // 1 is used to read a word from the EEPROM. 
	}
	else
	{
		counter = 0;											// If nothing was stored, then we will restart the counter from 0. 
	}
	
}

ISR(WDT_vect)		// Every 4 seconds, this will be called. We will store the current counter and flag value inside the EEPROM. 
{
   //eeprom_write_dword((uint32_t*)1,counter);			// Store the current counter value.
   eeprom_write_byte((uint32_t*)0,'T'); 				// Store the flag so that when we read it again from reset, we can continue the counter. 
}

void ADC_Convert()
{
	ADCSRA |= 1<<ADSC;								    // Conversion begins.
	 
	while((ADCSRA & (1<<ADSC)));                        // Wait till conversion has finished.
	
	Ain = ADCL;											// Read lower bits.
	Ain |= ADCH<<8;										// Read the upper bits.
	
	Voltage = (float)Ain/1024.00 * 5.00;				// Decimal value for voltage.
	
	// Check if the voltage is below 4V, if so we will reset the timer for the watchdog to interrupt if the condition of 4 seconds is met. 
	if(Voltage<4.00) 
	wdt_reset();
	
}

void save_in_memory(int c)
{
	eeprom_write_dword((uint32_t*)1,c);			// Store the current counter value.
}


void print_UART(char* c)
{
	fprintf(stdout, c);
}

int main(void)
{
	// This has to be done every-time you use a watch-dog.
	wdt_reset();
	MCUSR &= ~(1<<WDRF);
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	WDTCSR = 0x00;							// This will disable the watchdog. 
	
    initialize_all();
	LCDclr();
	
	LCDGotoXY(0,0);
	printString("Voltage: ");
	
	LCDGotoXY(0,1);
	printString("Counter: "); 
	
	current_time = 0;
	sei();
	
	
	previous_state_EEPROM();
	
    LCDGotoXY(9,1);
	sprintf(buffer, "%d", counter);
	printString(buffer);
	
	fprintf(stdout, "Starting...\n\r"); 
	
    while (1) 
    {
	  	ADC_Convert();
		dtostrf(Voltage, 4, 3, buffer);
		LCDGotoXY(9,0);
		printString(buffer);
		
		_delay_ms(50);
		current_time += 50;
		
		if(current_time >= 1000)
		{
		
		counter++;
		sprintf(buffer,"%d", counter);
		save_in_memory(counter);
		LCDGotoXY(9,1);
		printString(buffer);
		current_time = 0;  	
			
		}
		  
    }
}

