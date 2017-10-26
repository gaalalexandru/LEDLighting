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

//Select timer0 (8 bit) to use for PWM generation
//Select timer1 (16 bit) to use for delay functionality
//No function for timer2 (8 bit)

volatile uint16_t global_delay_ms = 0;

/************************************************************************/
/*	                Timer Initialization Functions                      */
/************************************************************************/

/* Timer0 - 8bit Initialization function*/
void timer0_init(void)
{
	TCCR0 = (1 << CS00);  //no prescaling (timer0 clock = system clock)
	TCNT0 = 0;
	TIMSK  |= (1 << TOIE0);  //Timer0 Overflow Interrupt Enable
}

/* Timer1 - 16bit Initialization function*/
void timer1_init(void)
{
	TCCR1A = 0;
	//Clear Timer on Compare mode and no prescaling (timer0 clock = system clock)
	TCCR1B = (1 << WGM12)|(1 << CS01)|(1 << CS00);
	TCNT1 = 0;
	//125 clock cycles is equivalent to 1 ms with the following setup:
	//system clock 8 Mhz
	//timer1 clock prescaler (divider) = 64 => timer1 clock 125 kHz
	//8000000 / 64 = 125000 (1 second)
	// 125000 / 1000 = 125 (1 mili second)
	OCR1A = 125;  //1 ms
	TIMSK  |= (1 << OCIE1A);  //Timer1 Output Compare A Match Interrupt Enable
}

/* Timer1 - 16bit De-Initialization function*/
void timer1_deinit(void)
{
	TIMSK  &= ~(1 << OCIE1A);  //Timer1 Output Compare A Match Interrupt Disable
	TCCR1B = 0;  //No clock
	TCNT1 = 0;	//Reset counter
}

#if 0
/* Timer2 - 8bit Initialization function*/
void timer2_Init(void)
{
	TCCR2 = (1 << WGM21)|(1 << CS01)|(1 << CS00);  //prescaler = 32, CTC mode, OC pin disabled
	TCNT2 = 0;
	OCR2 = 125;
	TIMSK  |= (1 << OCIE2);  //Enable Timer1 output compare trigger OCIE1A
}
#endif  //USE_TIMER2

/************************************************************************/
/*	                  Timer Delay / Wait Functions                      */
/************************************************************************/
/* Timer1 - millisecond wait function*/
inline void timer_delay_ms(uint16_t delay)
{
	//To increase time accuracy. 
	//Lack of accuracy of ~ 3.5% possible due to internal oscillator
	delay = 0.97 * delay;
	timer1_init();
	while(global_delay_ms < delay)
	{
		//just wait
	}
	timer1_deinit();
	global_delay_ms = 0;  //reset
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
	global_delay_ms++; //increment every 1 ms
}

#if 0
/* Timer2 Interrupt function*/
ISR (TIMER2_COMP_vect)
{
	TOGGLE_STATUS_LED;
}
#endif  //USE_TIMER2




