/*
 * LabTest2Practice.c
 *
 * Created: 9/19/2016 10:12:57 PM
 * Author : Jay
 *
 * Problem #2 
 *
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#define SWITCH1_PUSHED  !(PINB & (1<<PINB7)) // First Switch
#define SWITCH2_PUSHED  !(PINB & (1<<PINB2)) // Second Switch
#include <string.h>
#include "uart.h"

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar,uart_getchar,_FDEV_SETUP_RW);
char arr[10];  // Holder for the pin. 
char reciever[10];  // Holder for the received PIN.
int toggle = 0;
int error = 0;  

int main(void)
{
	DDRB = 1<<DDB5;       // Turn on the light for the bank B.
	uart_init();
	stdout = stdin = stderr = &uart_str;   
	fprintf(stdout,"Please create a four digit PIN: \n");   // System turns on, asks the user for a 4 digit PIN. 
	fscanf(stdin,"%s",arr);                            // System initialized and PIN is stored in the array.
	fprintf(stdout, "You're pin has been stored. \n"); 
    while (1) 
    {
		if(SWITCH2_PUSHED && toggle==1){   // If the second switch is pressed, logout.
			toggle = 0;		     // User logs out of the system.
			fprintf(stdout,"You have logged off. \n");
		}
		
		if(SWITCH1_PUSHED && toggle==0)					// If the user is not logged in (toggle == 0) and the switch is pushed.
		{
		  fprintf(stdout,"Enter password: ");
		  fscanf(stdin,"%s",reciever);
		  
		  for(int i = 0; i<4; i++)
		  {
			  if(reciever[i] != arr[i])
			  {
				  error = 1;
				  break;
			  }
		  }
		  
		  if(error==0)
		  {
			  toggle = 1;
			  fprintf(stdout, "You have logged in to the system. \n");
		  }
		  else
		    fprintf(stdout, "Incorrect password. Get lost. \n");
		}
		else if(SWITCH1_PUSHED && toggle==1)
		 fprintf(stdout, "You are already logged on! \n");
		 
		
		
		if(toggle == 1)
		{
			PORTB = 1<<PORTB5;
			_delay_ms(200);
			PORTB = 0;
			_delay_ms(200);
		} 
			
		error = 0; // Reset error.
			
    }
}

