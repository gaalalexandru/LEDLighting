/*
 * Main.cpp
 *
 * Created: 10/4/2017 10:48:34 AM
 *  Author: dan.filip
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "usart_handler.h"

#define F_CPU 1000000UL

#define Pin0 0x01
#define Pin1 0x02
#define Pin2 0x04
#define Pin3 0x08
#define Pin4 0x10
#define Pin5 0x20
#define PIN6 0x40
#define PIN7 0x80

#define FOSC 1000000// Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

/*void USART_Init( unsigned int ubrr);
void USART_Transmit( unsigned char data );
unsigned char USART_Receive( void );*/
//extern void USART_OutChar( unsigned char data );

int main(void)
{
	unsigned char receiveData = 0;
	PORTB=0x00;
	PORTC=0x00;
	PORTD=0x00;
	DDRB=0xFF;
	DDRC=0xFF;
	DDRD=0xFF;
	USART_Init(MYUBRR);
	USART_NewLine();
	USART_OutString("Init Done");
    while(1)
    {
		receiveData = USART_InChar();
		if(receiveData == 0x31) 
		{
			USART_NewLine();
			USART_OutString("Starting LED strip: ");
			USART_OutChar(receiveData);
			PORTB = 0x00;
			PORTC = 0x00;
			PORTD = PIN6;

		}
		else if (receiveData == 0x32) 
		{
			USART_NewLine();
			USART_OutString("Starting LED strip: ");
			USART_OutChar(receiveData);
			PORTB = 0x00;
			PORTC = 0x00;
			PORTD = Pin5;

		}
		else if (receiveData == 0x33) 
		{
			USART_NewLine();
			USART_OutString("Starting LED strip: ");
			USART_OutChar(receiveData);
			PORTB = Pin0;
			PORTC = 0x00;
			PORTD = 0x00;

		}
		else if(receiveData == 0x34) 
		{
			USART_NewLine();
			USART_OutString("Starting LED strip: ");
			USART_OutChar(receiveData);
			PORTB = 0x00;
			PORTC = 0x00;
			PORTD = PIN7;
		}
		else if (receiveData == 0x35) 
		{
			USART_NewLine();
			USART_OutString("Starting LED strip: ");
			USART_OutChar(receiveData);
			PORTB = Pin2;
			PORTC = 0x00;
			PORTD = 0x00;

		}
		else if (receiveData == 0x36) 
		{
			USART_NewLine();
			USART_OutString("Starting LED strip: ");
			USART_OutChar(receiveData);
			PORTB = Pin1;
			PORTC = 0x00;
			PORTD = 0x00;
		}
		else
		{
			USART_NewLine();
			USART_OutString("All LED Off");
			PORTB=0x00;
			PORTC=0x00;
			PORTD=0x00;
		}
		//USART_Transmit(0x31);  //1 character
		
        //TODO:: Please write your application code 
		/*
		PORTB=0x00;
		PORTC=0x00;
		PORTD=0x00;
		_delay_ms(1500);
		PORTB=0xFF;
		PORTC=0xFF;
		PORTD=0xFF;
		_delay_ms(100);
		PORTB=0x00;
		PORTC=0x00;
		PORTD=0x00;
		_delay_ms(1500);
		PORTB=0xFF;
		PORTC=0xFF;
		PORTD=0xFF;
		_delay_ms(100);
		*/
		
    }
} 
