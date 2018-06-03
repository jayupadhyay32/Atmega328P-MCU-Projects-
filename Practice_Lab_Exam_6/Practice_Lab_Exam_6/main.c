/*
 * Practice_Lab_Exam_6.c
 *
 * Created: 11/15/2016 7:13:23 PM
 * Author : Jay
 
  ****** NON BLOCKING SPI *******
 
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "lcd_lib.h"

// SPI 
#define DDR_SPI DDRB
#define SPI_SS   2
#define SPI_MOSI 3
#define SPI_MISO 4
#define SPI_SCK  5

// Variables

volatile unsigned int Ain;
volatile uint8_t data_byte;
volatile int sw_counter = 1000;


// LCD Strings

char buffer[17];




void init_all()
{
	// Configure the LCD, Timer1, ADC, and SPI
	
	// Configure the LCD
	initialize_LCD();
	LCDcursorOFF();
	LCDclr();
	
	// Configure Timer1 [100ms CTC, Overflow @ 100ms]     OCRA = t*fcpu/ps  - 1        W/ PS of 64, we get OCR1A = 24999
	   TCCR1B |= (1<<WGM12); 
	   OCR1A = 24999;
	   TCCR1B |= (1<<CS11) | (1<<CS10);
	   TIMSK1 = 1<<OCIE1A;
	
	// Configure ADC
	   ADMUX  |= (1<<MUX2)  | (1<<MUX1);
	   ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); // Set ADC pre-scaler to 128
	   ADCSRA |= (1<<ADEN); // Enable ADC circuit
	   ADCSRA |= (1<<ADIE); // ADC Interrupt Enabled
	
	
	/* SLEEP MODE
	
	//init the A to D converter
	ADMUX = 0b00000001;
	ADCSRA = (1<<ADEN) | (1<<ADIE) + 7 ;
	SMCR = (1<<SM0) ; // sleep -- choose ADC mode
	
	*/
	
	//Configure SPI
	
	// SPI Initialization
	DDR_SPI |= (1<<SPI_SS)|(1<<SPI_MOSI)|(1<<SPI_SCK); // Configure outputs
	SPCR |= (1<<SPE)|(1<<MSTR); // Enable SPI, Master Mode
	SPCR |= (1<<SPR1)|(1<<SPR0); // Set clock rate fck/128
	SPCR |= (1<<SPIE); // SPI Interrupt Enabled
    // sleep_enable();
	
}

ISR(TIMER1_COMPA_vect) // Every 100ms, we will call this ISR to take a voltage value. 
{
	/*sprintf(buffer, "%f ", (data_byte/256)*5);
	LCDGotoXY(8,1);
	LCDstring((uint8_t*)buffer,strlen(buffer));*/
	
	volatile float voltage = (float)Ain ;
	voltage = (voltage/1024.0)*5.00 ; //(fraction of full scale)*Vref
	dtostrf(voltage, 6, 3, buffer);
	LCDGotoXY(8,1);
	LCDstring((uint8_t*)buffer,strlen(buffer));
	
	// Start A to D conversion
	ADCSRA |= (1<<ADSC);		/*if(sw_counter>=0)	sw_counter-=100;	*/}

ISR(ADC_vect)      // This ISR is called when conversion is completed. 
{
	// Read the ADC value
	Ain = (ADCL); // First read lower byte
	Ain |= (ADCH<<8); // Then read upper byte
	
	/*
	AinLow = (int)ADCL;
	Ain = (int)ADCH*256;
	Ain = Ain + AinLow;
	*/
	
	// Initiate SPI Transmission
	PORTB &= ~(1<<SPI_SS); // Pull Slave_Select low
	SPDR = (Ain>>2);       // Start transmission
}

ISR(SPI_STC_vect)
{
	PORTB |= (1<<SPI_SS);        // Pull Slave Select High
	data_byte = SPDR;            // Read received data
	
	/* Print on LCD */
	sprintf(buffer, "%u   ", data_byte);
	LCDGotoXY(8,0);
	LCDstring((uint8_t*)buffer, strlen(buffer));	}



int main(void)
{
    init_all();
	sei(); 
	LCDGotoXY(0,0);
	LCDstring("Master:", 7);
	
	sprintf(buffer, "%d ", data_byte);
	LCDGotoXY(0,1);
	LCDstring("Voltage: ",9);
	
	
	
    while (1) 
    {
	/*	if(sw_counter==0)
		{
			sprintf(buffer, "%d ", data_byte);
			LCDGotoXY(8,1);
			LCDstring((uint8_t*)buffer,strlen(buffer));
			
			sw_counter = 1000;
		}
	*/	
	/*	sprintf(buffer, "%d ", data_byte);
		LCDGotoXY(8,1);
		LCDstring((uint8_t*)buffer,strlen(buffer));*/
		
	/*	//The sleep statement lowers digital noise and starts the A/D conversion
		sleep_cpu();				//program ONLY gets here after ADC ISR is done
		voltage = (float)Ain ;
		voltage = (voltage/1024.0)*Vref ; //(fraction of full scale)*Vref
		dtostrf(voltage, 6, 3, v_string);
		printf("%s", v_string);	*/
		
		
	/*	LCDGotoXY(8,1);		dtostrf((float)Ain/256 * 5.00, 4, 3, kwak);		LCDstring((uint8_t*)kwak,strlen(kwak));
		
		_delay_ms(100);
	*/	
    }
}

