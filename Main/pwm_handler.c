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

#include "configuration.h"


//macros to set pin assigned to pwm channels
//if layout changes update the macros also
#define PWM_SET_CH0(x) (PORTD = (x << PIN6))
#define PWM_SET_CH1(x) (PORTD = (x << PIN5))
#define PWM_SET_CH2(x) (PORTB = (x << PIN0))
#define PWM_SET_CH3(x) (PORTD = (x << PIN7))
#define PWM_SET_CH4(x) (PORTB = (x << PIN2))
#define PWM_SET_CH5(x) (PORTB = (x << PIN1))

//global buffers
uint8_t compare[CHMAX];
volatile uint8_t compbuff[CHMAX];

/*
 * Setting all pins individually allows the user to freely select pins regardless of port,
 * but it is not optimal for minimizing jitter.
 *
 * To lower the jitter, replace this function with code that sets the complete port(s)
 * at the same time. - DONE
 */
static void pwm_set_all_chanels(const bool level)
{
	/*
	PWM_SET_CH0(level);
	PWM_SET_CH1(level);
	PWM_SET_CH2(level);
	PWM_SET_CH3(level);
	PWM_SET_CH4(level);
	PWM_SET_CH5(level);
	*/
	//PORTB = (level << PIN3) | (level << PIN5) | (level << PIN6);  //stupid mistake
	PORTB = (level << PIN0) | (level << PIN1) | (level << PIN2);

	//PORTD = (level << PIN1) | (level << PIN2) | (level << PIN4);  //stupid mistake
	PORTD = (level << PIN5) | (level << PIN6) | (level << PIN7); 
}

void pwm_init(void)
{
	uint8_t i, pwm;
	pwm = PWMDEFAULT;
	/*for (i = 0; i < CHMAX; i++) // initialise all channels
	{
		compare[i]  = pwm; // set default PWM values
		compbuff[i] = pwm; // set default PWM values
	}*/
	compare[1]  = 1;
	compbuff[1] = 1;
	
	compare[0]  = 0x00;
	compbuff[0] = 0x00;
	
	compare[3]  = 1;
	compbuff[3] = 1;

	compare[2]  = 0x00;
	compbuff[2] = 0x00;
	
	compare[5]  = 1;
	compbuff[5] = 1;	

	compare[4]  = 0x00;
	compbuff[4] = 0x00;	
}

void pwm_update(void)
{
	static uint8_t softcount = 0xFF;

	/* increment modulo 256 counter and update
	the compare values only when counter = 0.
	verbose code for speed
	last element should equal CHMAX - 1 */
	
	if (++softcount == 0) {       
		compare[0] = compbuff[0]; 
		compare[1] = compbuff[1];
		compare[2] = compbuff[2];
		compare[3] = compbuff[3];
		compare[4] = compbuff[4];
		compare[5] = compbuff[5]; 
		pwm_set_all_chanels(true);
	}
	// clear port pin on compare match (executed on next interrupt)
	if (compare[0] == softcount) 
		PWM_SET_CH0(false);
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

				