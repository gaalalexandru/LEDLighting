/*
 * Main.cpp
 *
 * Created: 10/4/2017 10:48:34 AM
 *  Author: dan.filip
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "configuration.h"
#include "usart_handler.h"
#include "timer_handler.h"
#include "pwm_handler.h"


#define FOSC 1000000// Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1



int main(void)
{
	uint8_t receiveData = 0;
	PORTB=0x00;
	PORTC=0x00;
	PORTD=0x00;
	DDRB=0xFF;
	DDRC=0xFF;
	DDRD=0xFF;
	//USART_Init(MYUBRR);
	timer0_init();
	pwm_init();
	sei();  // enable global interrupts
	//USART_NewLine();
	//USART_OutString("Init Done");
    while(1)
    {
		//receiveData = USART_InChar();
    }
}