/*
 * pwm_handler.c
 *
 * Created: 10/10/2017 10:30:14 PM
 *  Author: Alexandru Gaal
 */ 
#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/portpins.h>

#include "pwm_handler.h"
#include "configuration.h"

//macros to set pin assigned as pwm channels
//if layout changes update the macros also
#define PWM_SET_CH0(x)	set_port_d(6,x)
#define PWM_SET_CH1(x)	set_port_d(5,x)
#define PWM_SET_CH2(x)	set_port_b(0,x)
#define PWM_SET_CH3(x)	set_port_d(7,x)
#define PWM_SET_CH4(x)	set_port_b(2,x)
#define PWM_SET_CH5(x)	set_port_b(1,x)

//global buffers
uint8_t pwm_width[CHMAX];
volatile uint8_t pwm_width_buffer[CHMAX];

//Set PORT level true = high, false = low
//pin = pin number to be set / cleared
static inline void set_port_b(const uint8_t pin, const bool level)
{
	if (level)
	{
		PORTB |= 1 << pin;
	}
	else
	{
		PORTB &= ~(1 << pin);
	}
}

static inline void set_port_d(const uint8_t pin, const bool level)
{
	if (level)
	{
		PORTD |= 1 << pin;
	}
	else
	{
		PORTD &= ~(1 << pin);
	}
}

/*
 * Setting all pins individually allows the user to freely select pins regardless of port,
 * but it is not optimal for minimizing jitter.
 *
 * To lower the jitter, replace this function with code that sets the complete port(s)
 * at the same time. - DONE
 */
static void pwm_set_all_chanels(const bool level)
{
	PORTB = (level << PIN0) | (level << PIN1) | (level << PIN2);
	PORTD = (level << PIN5) | (level << PIN6) | (level << PIN7); 
}

void pwm_init(void)
{
	uint8_t i, pwm;
	pwm = PWMDEFAULT;
	for (i = 0; i < CHMAX; i++) // initialise all channels
	{
		pwm_width[i]  = pwm; // set default PWM values
		pwm_width_buffer[i] = pwm; // set default PWM values
	}
	pwm_set_all_chanels(false);

}

void pwm_update(void)
{
	//static uint8_t softcount = 0xFF;
	static uint8_t softcount = 0x00;
	/* increment modulo 256 counter and update
	the pwm_width values only when counter = 0.
	verbose code for speed, do not replace with for...
	last element should equal CHMAX - 1 */
	softcount++;
	if (softcount == 0)
	{
		pwm_width[0] = pwm_width_buffer[0]; 
		pwm_width[1] = pwm_width_buffer[1];
		pwm_width[2] = pwm_width_buffer[2];
		pwm_width[3] = pwm_width_buffer[3];
		pwm_width[4] = pwm_width_buffer[4];
		pwm_width[5] = pwm_width_buffer[5];
		
		//current method fast enough and can check 0% channel setting
		//pwm_set_all_chaels function is not needed for the moment
		//has the drawback of not being able to set duty cycle 0%
		//pwm_set_all_chanels(true);
		PWM_SET_CH0((pwm_width[0] > 0));
		PWM_SET_CH1((pwm_width[1] > 0));
		PWM_SET_CH2((pwm_width[2] > 0));
		PWM_SET_CH3((pwm_width[3] > 0));
		PWM_SET_CH4((pwm_width[4] > 0));
		PWM_SET_CH5((pwm_width[5] > 0));					
	}
	
	// clear port pin on pwm_width match
	if (pwm_width[0] == softcount)
		PWM_SET_CH0(false);
	if (pwm_width[1] == softcount)
		PWM_SET_CH1(false);
	if (pwm_width[2] == softcount)
		PWM_SET_CH2(false);
	if (pwm_width[3] == softcount)
		PWM_SET_CH3(false);
	if (pwm_width[4] == softcount)
		PWM_SET_CH4(false);
	if (pwm_width[5] == softcount)
		PWM_SET_CH5(false);
}

				