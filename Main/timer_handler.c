/*
 * timer_handler.c
 *
 * Created: 10/5/2017 9:31:23 PM
 *  Author: Gaal Alexandru
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer_handler.h"
#include "pwm_handler.h"

/* Timer0 - 8bit Initialization function*/
void timer0_init(void)
{
	TCCR0 = (1 << CS00);  //clk/1024 (From prescaler)
	TCNT0 = 0;
	TIMSK  |= (1 << TOIE0);  //Enable Timer0 overflow interrupt TOIE0
}

/* Timer1 - 16bit Initialization function*/
void timer1_init(void)
{
	TCCR1A = 0;
	TCCR1B = (1 << WGM12)|(1 << CS00);  //clk/254 (From prescaler)
	TCNT1 = 0;
	OCR1A = 500;
	TIMSK  |= (1 << OCIE1A);  //Enable Timer1 output compare trigger OCIE1A
}

/* Timer2 - 8bit Initialization function*/
void timer2_Init(void)
{
	TCCR2 = (1 << WGM21)|(1 << CS01)|(1 << CS00);  //prescaler = 32, CTC mode, OC pin disabled
	TCNT2 = 0;
	OCR2 = 125;
	TIMSK  |= (1 << OCIE2);  //Enable Timer1 output compare trigger OCIE1A
}
/*
unsigned int TIM16_ReadTCNT1( void )
{
	uint8_t sreg;
	unsigned int i;
	sreg = SREG;  // Save Global Interrupt Flag 
	_CLI();//Disable interrupts
	i = TCNT1;	// Read TCNT1 into i
	SREG = sreg;  // Restore Global Interrupt Flag
	return i;
}
*/

/* Timer2 Compare MatchInterrupt function*//*
ISR (TIMER2_COMP_vect)
{
	uint8_t pulse_width = 10;
	static uint8_t counter = 0;
	if (counter <= pulse_width)
	{
		PORTD = Pin6;
	}
	else
	{
		PORTD = 0x00;
	}
	counter = (counter+1)%100;
}*/

/* Timer1 Compare Match A Interrupt function*/
/*
ISR (TIMER1_COMPA_vect)
{
	static uint8_t ledState = 0;
	if(ledState)
	{
		PORTD = Pin5;
	}
	else
	{
		PORTD = Pin6;
	}
	ledState ^= 1;
}
*/

/* Timer0 Overflow Interrupt function*/

ISR (TIMER0_OVF_vect)
{
	pwm_update();
}