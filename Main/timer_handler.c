 /*
 * timer_handler.c
 *
 * Created: 10/5/2017 9:31:23 PM
 *  Author: Gaal Alexandru
 */

//Select timer0 (8 bit) to use for PWM generation
//Select timer1 (16 bit) to toggle status LED
//Select timer2 (8 bit) to use for millisecond counter & delay functionality

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "configuration.h"
#include "timer_handler.h"
#include "pwm_handler.h"

/************************************************************************/
/*	                          Global Variables                          */
/************************************************************************/

volatile uint32_t timer_system_ms = 0;
volatile uint32_t timer_counter_target_ms = 0;

/************************************************************************/
/*	                  Timer Initialization Functions                    */
/************************************************************************/

/* Timer0 - 8bit Initialization function*/
void timer0_init(void)
{
	TCCR0 = (1 << CS00);  //no prescaler (timer0 clock = system clock)
	TCNT0 = 0;
	TIMSK  |= (1 << TOIE0);  //Timer0 Overflow Interrupt Enable
}

/* Timer1 - 16bit Initialization function*/
void timer1_init(void)
{
	TCCR1A = 0;
	//Clear Timer on Compare mode and /1024 prescaler
	//timer1 clock = system clock / 1024
	TCCR1B = (1 << WGM12)|(1 << CS12)|(1 << CS10);
	TCNT1 = 0;
	//3906 clock cycles is equivalent to 0.5 s with the following setup:
	//system clock 8 Mhz
	//timer1 clock prescaler (divider) = 1024 => timer1 clock 7812.5 Hz
	//8000000 / 1024 = 7812.5 (1 second)
	//7812.5 / 2 = 3906.25 (0.5 second)
	OCR1A = 3906;  //0.5 seconds
	TIMSK  |= (1 << OCIE1A);  //Timer1 Output Compare A Match Interrupt Enable
}

/* Timer2 - 8bit Initialization function*/
void timer2_init(void)
{
	//Clear Timer on Compare mode and /64 prescaler, OC pin disabled
	//timer2 clock = system clock / 64
	TCCR2 = (1 << WGM21)|(1 << CS22);
	TCNT2 = 0;
	//125 clock cycles is equivalent to 1 ms with the following setup:
	//system clock 8 Mhz
	//timer1 clock prescaler (divider) = 64 => timer1 clock 125 kHz
	//8000000 / 64 = 125000 (1 second)
	// 125000 / 1000 = 125 (1 mili second)
	OCR2 = 125;
	TIMSK  |= (1 << OCIE2);  //Enable Timer1 output compare trigger OCIE2
}

/************************************************************************/
/*	                 Timer Delay / Counter Functions                    */
/************************************************************************/
/* Millisecond wait function*/
void timer_delay_ms(uint32_t delay)
{
	//HINT: To increase time accuracy use a 0.97 coefficient on target_ms or delay 
	//Lack of accuracy of ~ 3.5% possible due to internal oscillator
	uint32_t target_ms = timer_ms() + (uint32_t)delay;
	while(timer_ms() < target_ms) { /*Wait*/ }
}

/* Millisecond counter function since system start-up*/
inline uint32_t timer_ms(void)
{
	/*uint32_t currMillis;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		currMillis = timer_system_ms;
	}*/
	return timer_system_ms/*timer_system_ms*/;
}

/* Passive counter / delay setup function*/
/* This will not interrupt the program and will not loop*/
/* will only set a flag when counter value expider*/
void timer_counter_setup(uint16_t second)
{
	timer_counter_target_ms = timer_ms() + (second * 1000);
}

/************************************************************************/
/*	               Timer Interrupt Service Routines                     */
/************************************************************************/

/* Timer0 Overflow Interrupt function*/
ISR (TIMER0_OVF_vect)
{
	pwm_update();
}

/* Timer1 Compare Match A Interrupt function*/
ISR (TIMER1_COMPA_vect)
{
	#if STATUS_LED
	TOGGLE_STATUS_LED;
	#endif
}

/* Timer2 Interrupt function*/
ISR (TIMER2_COMP_vect)
{
	timer_system_ms++; //increment every 1 ms
}




