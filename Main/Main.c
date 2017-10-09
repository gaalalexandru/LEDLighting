/*
 * Main.cpp
 *
 * Created: 10/4/2017 10:48:34 AM
 *  Author: dan.filip
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart_handler.h"
#include "timer_handler.h"

#define Pin0 0x01
#define Pin1 0x02
#define Pin2 0x04
#define Pin3 0x08
#define Pin4 0x10
#define Pin5 0x20
#define Pin6 0x40
#define Pin7 0x80

#define FOSC 1000000// Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1


int main(void)
{
	unsigned char receiveData = 0;
	PORTB=0x00;
	PORTC=0x00;
	PORTD=0x00;
	DDRB=0xFF;
	DDRC=0xFF;
	DDRD=0xFF;
	//USART_Init(MYUBRR);
	TIMER2_Init();
	sei();  // enable global interrupts
	USART_NewLine();
	USART_OutString("Init Done");
    while(1)
    {
		receiveData = USART_InChar();
		//LedChainTest(receiveData);

    }
}


static void LedChainTest(unsigned char ledNr)
{
	if(ledNr == 0x32)
	{
		PORTB = 0x00;
		PORTC = 0x00;
		PORTD = Pin5;
		
	}
	else if (ledNr == 0x31)
	{
		PORTB = 0x00;
		PORTC = 0x00;
		PORTD = Pin5;
		
	}
	else if (ledNr == 0x34)
	{
		PORTB = Pin0;
		PORTC = 0x00;
		PORTD = 0x00;
		
	}
	else if(ledNr == 0x33)
	{
		PORTB = 0x00;
		PORTC = 0x00;
		PORTD = Pin7;
	}
	else if (ledNr == 0x36)
	{
		PORTB = Pin2;
		PORTC = 0x00;
		PORTD = 0x00;
		
	}
	else if (ledNr == 0x35)
	{
		PORTB = Pin1;
		PORTC = 0x00;
		PORTD = 0x00;
	}
	else
	{
		PORTB=0x00;
		PORTC=0x00;
		PORTD=0x00;
	}
}