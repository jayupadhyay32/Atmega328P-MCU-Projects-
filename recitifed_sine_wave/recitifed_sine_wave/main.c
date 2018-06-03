
#define F_CPU 16000000UL
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_TICKS 249			// Timer1 Ticks Corresponding to 64kHz period

// PWM variables
volatile uint16_t duty_cycle;
volatile uint16_t time_period;
volatile uint8_t toggle_flag;
int percentage_duty_cycle;
volatile long time;
volatile float a;

// All initializations
void initialize_all(void)
{
	// Setting up pins as output
	DDRB |= (1<<DDB2);		// PB2 (OC1B)
	a = 0;
	time = 0;
	// Initializing counters
	time_period = MAX_TICKS; // 64kHz
	duty_cycle = 0;
	
	// Setup Timer1
	OCR1A = time_period;				// Set the compare reg A
	OCR1B = duty_cycle;					// Set the compare reg B
	TCCR1A |= (1<<WGM11) | (1<<WGM10);	// turn on Fast PWM mode
	TCCR1B |= (1<<WGM13) | (1<<WGM12);	// turn on Fast PWM mode
	TCCR1A |= (1<<COM1B1);				// Set OC1B pin (PB2) to clear on compare match
	TIMSK1 |= (1<<OCIE1A);				// turn on Timer1 Compare Match A ISR
	TCCR1B |= (1<<CS10);				// Set prescalar = 1
	
	// Setup Timer0
	OCR0A = 249;					//  1 Millisecond is 249 ticks
	TIMSK0 |= (1<<OCIE0A);			// turn on Timer0 Compare match ISR
	TCCR0A |= (1<<WGM01);			// turn on clear-on-match, CTC mode
	TCCR0B = (1<<CS00) | (1<<CS01); // Set pre-scalar to divide by 64
}

// Timer 1 Compare Match A ISR (TCNT1 = OCR1A)
ISR (TIMER1_COMPA_vect)
{
	// Load new Time period
	OCR1A = time_period;
	
	// Load new duty cycle
	OCR1B = duty_cycle;
}

ISR(TIMER0_COMPA_vect)
{
	time++;
	a = time*0.001;		//convert to actual time in ms
	duty_cycle = fabs(sin(2*M_PI*62.5*(a))) * time_period;
}

int main(void)
{
	initialize_all();
	sei(); // Enable global interrupts
	
	while (1)
	{
		// Nothing to do.
	}
}