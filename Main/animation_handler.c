/*
 * animation_handler.c
 *
 * Created: 3/7/2018 10:52:10 AM
 *  Author: bogdan.rat
 */ 

#include <stdbool.h>
#include "configuration.h"
#include "animation_handler.h"
#include "timer_handler.h"

#define true	1
#define false	0

extern volatile uint8_t pwm_width_buffer[CHMAX];

void animation_init(void)
{
	uint8_t i;
	#if (STARTUP_ANIMATION_MODE == 1)
	for(i = 0; i < CHMAX; ++i)
	{
		timer_delay_ms(100);
		pwm_width_buffer[i] = STARTUP_ANIMATION_DEFAULTPWM;
	}
	
	#elif (STARTUP_ANIMATION_MODE == 2)
	for(uint8_t j = PWM_DUTY_CYCLE_RESET_VALUE; j <= STARTUP_ANIMATION_DEFAULTPWM; ++j)
	{
		timer_delay_ms(100);
		for(i = 0; i < CHMAX; ++i)
		{
			pwm_width_buffer[i] = j;
		}
	}
	#endif
}

void animation_setallchannels(const bool level)
{
	for(uint8_t i = 0; i < CHMAX; ++i)
	{
		if(level)
		{
			pwm_width_buffer[i] = NOCONNECTION_ANIMATION_DEFAULTPWM;
		}
		else
		{
			pwm_width_buffer[i] = 0;
		}
	}
}
