/*
 * Lab_SPI_TASK1.c
 *
 * Created: 11/14/2016 9:52:14 PM
 * Author : Jay
 *
 *
 * Implementation of SPI using both Master and Slave. 
 *
 * Bank B is basically for the entire Slave/Master thing. 
 *
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcd_lib.h"

#define DDR_SPI    DDRB
#define SPI_SS     2
#define SPI_MOSI   3    // Master output slave input, at PINB3 
#define SPI_MISO   4    // Master input slave output, at PINB4 
#define SPI_SCK    5

volatile unsigned int Ain; 
int data_byte; 
char buffer[17];

void printString(char* c)
{
	for(int i = 0; i<strlen(c); i++)
	LcdDataWrite(c[i]);
}

void initialize_all()
{
  initialize_LCD();
  LCDGotoXY(0,0);
  printString("Master: ");
  LCDGotoXY(0,1);
  printString("Slave: ");

  // ADC setup 
  ADMUX |= (1<<MUX2) | (1<<MUX1);                   // ADCP6
  ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);   // PS = 128
  ADCSRA |= (1<<ADEN);                              // ADC is enabled. 
  
  DDR_SPI |= (1<<SPI_SS) | (1<<SPI_MOSI) | (1<<SPI_MISO);                  // SS, MOSI, SCK is also set. 
  SPCR |= (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPR0);                    // Enable SPI, Master, and clock rate is set. 
  	
}



void init_slave()
{
	DDR_SPI |= (1<<SPI_MISO);				  // MISO is output, others are inputs.
	SPCR |= (1<<SPE);                         // Enable SPI
}

void SPI_Slave_Transceiver(uint8_t cData)     // Only use this if you want to work as a slave. 
{
	
	if(PINB & (1<<SPI_SS))
	SPDR = cData;
	
	if((SPSR & (1<<SPIF)))
	data_byte = SPDR; 
}


uint8_t SPI_Master_Transceiver(uint8_t cData)
{
	PORTB &= ~(1<<SPI_SS);					  // Slave select is pulled low.
	SPDR = cData;                             // Data gets ready for transmission.
	
	while(!(SPSR & (1<<SPIF)));               // Wait till transmission is finished.
	PORTB |= (1<<SPI_SS);                     // Slave is pulled high.
	
	return SPDR;                              // The value is transmitted. 
}

void analog_to_digital()
{
	ADCSRA |= 1<<ADSC;                        // Analog to digital conversion.
	
	while((ADCSRA & (1<<ADSC)));              // Wait until the conversion is complete.
	
	Ain = (ADCL);
	Ain |= (ADCH<<8);
}

void print_master_slave()
{
	// Printing the value for the master.
	
	sprintf(buffer, "%d ", (Ain>>2));
	LCDGotoXY(8,0);
	LCDstring((uint8_t*)buffer, strlen(buffer));
	
	
	// Printing the value for the slave.
	
	sprintf(buffer, "%d ", data_byte); 
	LCDGotoXY(8,1);
	LCDstring((uint8_t*)buffer,strlen(buffer));
	
}

int main(void)
{
    initialize_all();
	sei();
	
    while (1) 
    {
		analog_to_digital();
		
		data_byte = SPI_Master_Transceiver(Ain>>2);        // Only use this if you want to work as a master.
		// SPI_Slave_Transceiver(Ain>>2);                     Only use this if you want to work as a slave.
		
		print_master_slave();
		
		_delay_ms(100); 
		
    }
}

