/*
 * timer_handler.c
 *
 * Created: 10/5/2017 9:31:23 PM
 *  Author: Gaal Alexandru
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer_handler.h"
#include "usart_handler.h"

#define Pin5 0x20
#define Pin6 0x40

/* Timer0 Initialization function*/
void TIMER0_Init(void)
{
	//Init the timer
	//TCCR0 |= (1 << CS00);  //clk (No prescaling)
	TCCR0 |= (1 << CS02)|(1 << CS00);  //clk/1024 (From prescaler)
	TCNT0 = 0;
	TIMSK  |= (1 << TOIE0);  //Enable Timer0 overflow interrupt TOIE0
}

/* Timer0 Overflow Interrupt function*/
ISR (TIMER0_OVF_vect) // timer0 overflow interrupt
{
	static unsigned char ledState = 0;
	USART_OutChar((ledState+0x30));
	if(ledState)
	{
		PORTB = 0x00;
		PORTC = 0x00;
		PORTD = Pin6;
	}
	else 
	{
		PORTB = 0x00;
		PORTC = 0x00;
		PORTD = 0x00;		
	}
	ledState ^= 1;
}