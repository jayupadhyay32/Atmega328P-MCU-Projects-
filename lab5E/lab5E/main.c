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
#include "uart.h"
#define Vref 5.00
#define buffer_size 100
#define r_buffersize 50
#define LCDtimer 150;
#define UARTtimer 200;
#define ADCtimernormal 1;
#define ADCtimersleep 10;

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW); 

volatile voltage_buffer[buffer_size];
volatile int voltage_index;
volatile int voltage_time;
volatile int task_LCDtimer; // LCD task, 150 ms initialization
volatile int task_UARTtimer; // UART task, 125 ms initialization
volatile int task_ADCtimer; // ADC task
volatile int Timer; // Maintain 1ms clock to do proper averaging of ADC measurements
volatile int8_t PrintFlag; // ADC measurement can only start when all the printing to the LCD screen and terminal is done
int mode = 0;  

char r_buffer[r_buffersize];
int r_index;
volatile char r_ready;

volatile double Sum1, Sum2, average, standard_deviation; 


void getstr(void)
{
	r_ready = 0; // Clear the ready flag.
	r_index = 0; // Clear the buffer. 
	
	UCSR0B |= (1<<RXCIE0); 
}

// UART ISR

ISR(USART_RX_vect)
{
	char r_char;
	r_char = UDR0;
	// Echo character back out over the system such that a human user can see this
	UDR0 = r_char;
	if (r_char != '\r') // compare to the enter character
	{
		if ((r_char == '\b') || (r_char == 127)) // 127 represents the DEL key
		{
			putchar(' '); // erase character on screen
			putchar('\b'); // backspace
			if (r_index>0) {--r_index;} // erase previously read character in r_buffer
		}
		else
		{
			r_buffer[r_index] = r_char;
			if (r_index < r_buffersize-1) {r_index++;} else {r_index = 0;}
		}
	}
	else
	{
		putchar('\n'); // new line
		r_buffer[r_index]=0; //strings are terminated with a 0
		r_ready = 1;
		UCSR0B ^=(1<<RXCIE0); // turn off receive interrupt
	}
}

// UART ISR ENDS 

void switch_mode()
{
  if(!strcmp(r_buffer, "yes"))
  {
	  if(mode==1)
	  mode = 0;					// Switch to normal mode.
	  else if(mode == 0)
	  mode = 1;					// Switch to sleep mode. 
  }
  
  if(mode == 0)
  fprintf(stdout, "Device in Normal mode. \n");
  
  if(mode == 1)
  fprintf(stdout, "Device in Sleep mode. \n");
  
  getstr(); 
	
}

volatile int Ain, AinLow;
volatile float Voltage;
volatile float Variance;
 

char LCD_Buffer[17];

void write_to_LCD()   // Print our values to the LCD. 
{
	// Write Moving Average and Standard Deviation values to the LCD screen.
	LCDGotoXY(6, 0);
	dtostrf(average,10,9,LCD_Buffer);
	LCDstring(LCD_Buffer, strlen(LCD_Buffer));
	LCDGotoXY(6, 1);
	dtostrf(standard_deviation,10,9,LCD_Buffer);
	LCDstring(LCD_Buffer, strlen(LCD_Buffer));
	// With numerical errors we can see no difference in sleep or normal mode.
	// Without numerical errors: sleep mode has a smaller standard deviation !!}


void ADC_Voltage_Measure()   // This will convert for you, for either mode. 
{
	int v_oldtime, v_ind_middle, v_ind_end, j;
	double volt, volt_index;
	
	
	if(mode == 0)
	{
		ADCSRA |= (1<<ADSC);
		int8_t flag = 1;
		while(flag != 0)
		{
			flag = ADCSRA & (1<<ADSC);
		}
	}
	else
	{
		int8_t flag = 1;
		
		while(flag != 0)
		{
			while (flag !=0) 
			{ 
				flag = (UCSR0A ^ (1<<TXC0)) & ((1<<RXC0) | (1<<TXC0)); 
			}
			// checks bit RXC0 equals 0 and bit TXC0 equals 1: everything is received and transmitted
			// one may still see an artifact: typing a character while in sleep mode only sends part of the character over
			// the backspace functionality helps the user to correct such an artifact.
			
			   sleep_cpu(); 
		}
		
	}
	
	int voltage_oldtime = voltage_time;
	voltage_time = Timer;
	
	v_ind_middle = volt_index + (voltage_time-v_oldtime)/2;
	v_ind_end = volt_index + (voltage_time-v_oldtime);
	voltage_index = voltage_buffer[voltage_index];
	j = voltage_index;
	while (j < v_ind_middle) { // Performs Interpolation for missing ADC samples
		j++;
		voltage_index = (j % buffer_size);
		volt = voltage_buffer[voltage_index];
		voltage_buffer[voltage_index] = voltage_index;
		Sum1 = Sum1 - volt + voltage_index;
		Sum2 = Sum2 - (volt*volt) + (voltage_index * voltage_index);
	}
	voltage_index = ((double)Ain/1024.00)*5.00;
	while (j < v_ind_end){ // Performs Interpolation for missing ADC samples
		j++;
		voltage_index = (j % buffer_size);
		Voltage = voltage_buffer[voltage_index];
		voltage_buffer[voltage_index] = voltage_index;
		Sum1 = Sum1 - volt + volt_index;
		Sum2 = Sum2 - (Voltage*Voltage) + (voltage_index * voltage_index);
	}
	// Precise calculation of statistics!!
	Sum1= 0.0;
	j=0;
	while (j<buffer_size)
	{
		Sum1 += voltage_buffer[j];
		j++;
	}
	average=Sum1/((double)buffer_size);
	Sum2=0.0;
	j=0;
	while (j<buffer_size)
	{
		Sum2 += (voltage_buffer[j]-average)*(voltage_buffer[j]-average);
		j++;
	}
	standard_deviation = sqrt(Sum2/((double)buffer_size));
}

// ADC Conversion is complete, obtain ADCL and ADCH values, reading ADCL FIRST!
ISR (ADC_vect)
{
	AinLow = (int)ADCL;
	Ain = (int)ADCH*256;
	Ain = (Ain + AinLow);
}

// Every 1mS. 1uS Interrupts were causing problems with print functions
ISR(TIMER0_COMPA_vect)
{
	if (task_LCDtimer > 0) {task_LCDtimer--;}
	if (task_UARTtimer > 0) {task_UARTtimer--;}
	if (task_ADCtimer > 0) {task_ADCtimer--;}
	Timer++;
}

void printString(char* c)
{
	for(int i = 0; i<strlen(c); i++)
	LcdDataWrite(c[i]);
}

void initialize_all()
{
	//init the A to D converter
	int j;
	
	DDRD = 0b11111111;  // All of PORTD is set to output.
	
	
	//set up timer 0 for 1 mSec ticks for 16MHz crystal
	OCR0A = 249; //set the compare reg to 250 time ticks
	TIMSK0= (1<<OCIE0A); //turn on timer 0 compare match ISR
	TCCR0B= (1<<CS01) | (CS00); //set pre-scalar to divide by 64
	TCCR0A= (1<<WGM01); // turn on clear-on-match
	
	
	
	
	
	TCCR1A |= (1<<WGM11) | (1<<WGM10) | (1<<COM1A1);				// SET PWM by setting values in A.
	TCCR1B |= (1<<WGM13) | (1<<WGM12);				                // SET PWM by setting values in B as well.
	TCCR1B |= (1<<CS10);				                            // Set the prescaler to 1024.
	OCR1A = 15999;									                // Set the tick target to 15624.    (16Mhz/(1024*(15624+1)) = 1Hz PWM
	OCR1B = 7813;                                                   // 50% duty rate. 15624*.50 = 7813
	
	TIMSK1 |= (1<<OCIE1A) | (1<<OCIE1B);
	
	
	// ADC Configuration
	ADMUX = 0x06; // Set to Aref. ADC results right adjusted. ADC6
	// ADC Enable, ADSC start conversion, AD Interrupt enabled, Prescaler of 128
	ADCSRA = (1<<ADEN) | (1<<ADIE) + 7;
    // Set sleep mode to ADCNRM
    SMCR = (1<<SM0); // Sleep -- Choose ADC Noise Reduction Mode.
    // When the sleep task is called, it will automatically start ADC conversion by setting ADSC bit.
    sleep_enable(); // Allow sleep commands

	// Initialize UART
	uart_init(); // Initialize UART
	stdout = stdin = stderr = &uart_str; // Set File outputs to point to UART stream
	//Initialize global variables for ADC measurements
	Timer =0;
	average = 0.00;
	standard_deviation = 0.00;
	Sum1 =0.00;
	Sum2 = 0.00;
	j=0;
	while (j<buffer_size) {voltage_buffer[j]=0.00;j++;}
	voltage_time = 0;
	voltage_index = 0;
	
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



int main(void)
{
    /* Replace with your application code */
	initialize_all();
	initialize_LCD();
	LCDcursorOFF();
	
	
	mode = 0; // { in normal mode
		fprintf(stdout,
		"Status Device: normal mode. Change mode? \n\r");
		getstr();
	
    while (1) 
    {
		if (task_ADCtimer == 0)
		{
			if (mode==0) {task_ADCtimer = ADCtimernormal;}
			if (mode==1) {task_ADCtimer = ADCtimersleep;}
			ADC_Voltage_Measure();
		}
		if (task_LCDtimer == 0)
		{
			task_LCDtimer = LCDtimer;
			write_to_LCD();
		}
		if (r_ready == 1)
		{
			switch_mode();
			// Sleep mode interrupts the emptying of the print buffer:
			// The sentence has 40 characters, each taking about 1ms
			if (mode==1) {task_ADCtimer += 40;} // instead of _delay_ms(40);
			// print buffer must be empty
		}
    }
	
	
	
}

