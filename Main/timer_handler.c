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

#define USE_TIMER0  //Select timer to use

#ifdef USE_TIMER0
/* Timer0 - 8bit Initialization function*/
void timer0_init(void)
{
	TCCR0 = (1 << CS00);  //clk/1024 (From prescaler)
	TCNT0 = 0;
	TIMSK  |= (1 << TOIE0);  //Enable Timer0 overflow interrupt TOIE0
}

/* Timer0 Overflow Interrupt function*/
ISR (TIMER0_OVF_vect)
{
	pwm_update();
}
#endif //USE_TIMER0

#ifdef USE_TIMER1
/* Timer1 - 16bit Initialization function*/
void timer1_init(void)
{
	TCCR1A = 0;
	TCCR1B = (1 << WGM12)|(1 << CS00);
	TCNT1 = 0;
	OCR1A = 500;
	TIMSK  |= (1 << OCIE1A);  //Enable Timer1 output compare trigger OCIE1A
}

/* Timer1 Compare Match A Interrupt function*/
ISR (TIMER1_COMPA_vect)
{

}
#endif  //USE_TIMER1

#ifdef USE_TIMER2
/* Timer2 - 8bit Initialization function*/
void timer2_Init(void)
{
	TCCR2 = (1 << WGM21)|(1 << CS01)|(1 << CS00);  //prescaler = 32, CTC mode, OC pin disabled
	TCNT2 = 0;
	OCR2 = 125;
	TIMSK  |= (1 << OCIE2);  //Enable Timer1 output compare trigger OCIE1A
}

/* Timer2 Compare MatchInterrupt function*/
ISR (TIMER2_COMP_vect)
{

}
#endif  //USE_TIMER2


