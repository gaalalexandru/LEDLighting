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
//#include "gpio_handler.h"

//macros to set pin assigned as pwm channels
//if layout changes update the macros also
#define PWM_SET_CH0(x)	set_port_d(6,x)
#define PWM_SET_CH1(x)	set_port_d(5,x)
#define PWM_SET_CH2(x)	set_port_b(0,x)
#define PWM_SET_CH3(x)	set_port_d(7,x)
#define PWM_SET_CH4(x)	set_port_b(2,x)
#define PWM_SET_CH5(x)	set_port_b(1,x)

/*
#define PWM_SET_CH0(x) ((x == 1)?(PORTD |= (x << PIN6)):(PORTD &= ~(x << PIN6)))
#define PWM_SET_CH1(x) ((x == 1)?(PORTD |= (x << PIN5)):(PORTD &= ~(x << PIN5)))
#define PWM_SET_CH2(x) ((x == 1)?(PORTB |= (x << PIN0)):(PORTB &= ~(x << PIN0)))
#define PWM_SET_CH3(x) ((x == 1)?(PORTD |= (x << PIN7)):(PORTD &= ~(x << PIN7)))
#define PWM_SET_CH4(x) ((x == 1)?(PORTB |= (x << PIN2)):(PORTB &= ~(x << PIN2)))
#define PWM_SET_CH5(x) ((x == 1)?(PORTB |= (x << PIN1)):(PORTB &= ~(x << PIN1)))
*/

//global buffers
uint8_t compare[CHMAX];
volatile uint8_t compbuff[CHMAX];

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
	pwm = 0x0F;
	for (i = 0; i < CHMAX; i++) // initialise all channels
	{
		compare[i]  = pwm; // set default PWM values
		compbuff[i] = pwm; // set default PWM values
	}
	pwm_set_all_chanels(false);

}

void pwm_update(void)
{
	//static uint8_t softcount = 0xFF;
	static uint8_t softcount = 0x00;
	/* increment modulo 256 counter and update
	the compare values only when counter = 0.
	verbose code for speed
	last element should equal CHMAX - 1 */
	softcount++;
	if (softcount == 0) {       
		compare[0] = compbuff[0]; 
		compare[1] = compbuff[1];
		compare[2] = compbuff[2];
		compare[3] = compbuff[3];
		compare[4] = compbuff[4];
		compare[5] = compbuff[5]; 
		pwm_set_all_chanels(true);
	}
	// clear port pin on compare match (executed on next interrupt)
	if (compare[0] == softcount) {
		PWM_SET_CH0(false);
	}
	/*else {
		PWM_SET_CH0(true);
	}*/
	if (compare[1] == softcount)
		PWM_SET_CH1(false);
	if (compare[2] == softcount)
		PWM_SET_CH2(false);
	if (compare[3] == softcount)
		PWM_SET_CH3(false);
	if (compare[4] == softcount)
		PWM_SET_CH4(false);
	if (compare[5] == softcount)
		PWM_SET_CH5(false);
}

				