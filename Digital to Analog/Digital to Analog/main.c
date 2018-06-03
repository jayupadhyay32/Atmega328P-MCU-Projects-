/*
 * Digital to Analog.c
 *
 * Created: 12/3/2016 11:39:05 PM
 * Author : Jay
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/io.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <math.h>
#include "lcd_lib.h"


// SPI FUNCTIONS

#define SPI_DDR    DDRB
#define SPI_PORT   PORTB
#define SPI_SS     PORTB2
#define SPI_MOSI   PORTB3
#define SPI_MISO   PORTB4
#define SPI_SCK    PORTB5

// MCP4921 DAC FUNCTIONS

#define SELECT_DAC_A     0x0000
#define SELECT_DAC_B     0x8000
#define VREF_UNBUFF      0x0000
#define VREF_BUFF        0x4000
#define GAIN_2X          0x0000
#define GAIN_1X          0x2000
#define OUTPUT_DISABLED  0x0000
#define OUTPUT_ENABLED   0x1000

// Utilities

#define DAC_DATA_MAX    4095
#define LDAC_PIN        PORTB0
#define LDAC_PIN_DDR    DDRB
#define LDAC_PIN_PORT   PORTB

volatile unsigned int Ain;
volatile uint16_t DAC_data;
volatile float pi = 3.14;
volatile int freq = 100; 

char buffer[17];

void printString(char* c, int a, int b)
{
	LCDGotoXY(a,b);
	
	for(int i = 0; i<strlen(c); i++)
	LcdDataWrite(c[i]);
}

void start_adc_wait()
{
	ADCSRA |= (1<<ADSC);
	
	// Wait until ADC conversion is completed. 
	while((ADCSRA & (1<<ADSC)));
}

void start_transmission_DAC()
{
	LDAC_PIN_PORT |= (1<<LDAC_PIN);    // Pull the LDAC pin to High.
	SPI_PORT &= ~(1<<SPI_SS);          // Pull the Slave select to Low.
}

void stop_transmission_DAC()
{
	SPI_PORT |= (1<<SPI_SS);          // Pull the Slave select to High.
	LDAC_PIN_PORT &= ~(1<<LDAC_PIN);  // Pull the LDAC to low. 
}

void SPI_Transmitter_DAC(uint16_t data)
{
	start_transmission_DAC();
	
	SPDR = (data >> 8);                // Send upper byte from the data to DAC.
	while(!(SPSR & (1<<SPIF)));        // Wait for transmission to complete.
	
	SPDR = (data & 0xFF);              // Send the lower bytes.
	while(!(SPSR & (1<<SPIF)));        // Wait until the transmission has finished.
	
	stop_transmission_DAC();

}

int time_counter = 0; 

void sine_compute_task()
{
	float value;
	
	value = 5*sin(freq*2*pi*time_counter);
	
	DAC_data = (value * 1.0 * DAC_DATA_MAX) / 1024.0; 
	
}


void initialize_all()
{
	initialize_LCD();
	LCDcursorOFF();
	LCDclr();
	printString("Master: ", 0, 0);
	printString("Slave:  ", 0, 1);
	
	// ADC Setup
	ADMUX |= (1<<MUX2) | (1<<MUX1);                      // Channel ADC 6
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);      // ADC Prescaler is set to 128.
	ADCSRA |= (1<<ADEN);                                 // ADC circuit is enabled.
	
	DDRB |= (1<<DDB0) | (1<<DDB2) | (1<<DDB3) | (1<<DDB5);
	
	// SPI inits.
	SPI_DDR |= (1<<SPI_SS) | (1<<SPI_MOSI) | (1<<SPI_SCK);
	SPCR  |= (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPR0);            // Enable SPI, Master, with fck/128.
	
	// DAC inits. 
	
	LDAC_PIN_DDR  |= (1<<DDB0);                                   // LDAC is set as output.
	LDAC_PIN_PORT |= (1<<LDAC_PIN);                                  // Initially LDAC pin is set to high. 
	
	sei();
}


int main(void)
{
    initialize_all();
	
	volatile float Voltage; 	
	
    while (1) 
    {
	  start_adc_wait();
	  
	  Ain = ADCL;             // Read lower bytes first.
	  Ain |= (ADCH<<8);       // Read the upper bytes.
	  
	  DAC_data = (Ain * 1.0 * DAC_DATA_MAX) / 1024.0;
	  Voltage = Ain*5.0/1024.0;
	  
	  //sine_compute_task();
	  DAC_data |= (SELECT_DAC_A | VREF_BUFF | GAIN_1X | OUTPUT_ENABLED);
	  
	  SPI_Transmitter_DAC(DAC_data);
	  //sprintf(buffer, "%u   ", Ain);
	  dtostrf(Voltage, 4, 3, buffer);
	  LCDGotoXY(8,0);
	  LCDstring((uint8_t*)buffer, strlen(buffer));
	  
	  //sprintf(buffer, "%u   ", (DAC_data & 0x0FFF));
	  dtostrf((DAC_data & 0x0FFF)/1024.0,4,3, buffer);
	  LCDGotoXY(8,1);
	  LCDstring((uint8_t*)buffer, strlen(buffer));
	  
	  _delay_ms(100);	
	  //time_counter += 100;	
		
    }
}

