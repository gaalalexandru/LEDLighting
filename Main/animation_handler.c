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
#include "eeprom_handler.h"
#include "pwm_handler.h"
#include "uart_handler.h"

#define true	1
#define false	0

extern volatile uint8_t pwm_width_buffer[PWM_CHMAX];

void animation_init(void)
{
	uint8_t i;
	uint8_t u8anim = 0;
	uint8_t u8power = 0;
	
	u8anim = animation_load_startup_anim();
	
	if(u8anim == ANIMATION_SUA_NONE)
	{
		//no startup animation
	}
	else if(u8anim == ANIMATION_SUA_CIRCLE)
	{
		//circle startup animation
		//1 = starts one led strip one after another
		u8power = pwm_load_default_dutycycle();
		for(i = 0; i < PWM_CHMAX; ++i)
		{
			timer_delay_ms(100);
			pwm_width_buffer[i] = u8power;
		}
		
	}
	else if(u8anim == ANIMATION_SUA_SMOOTH)
	{
		//smooth startup animation
		//2 = start all led strips with increasing intensity
		u8power = pwm_load_default_dutycycle();
		for(uint8_t j = PWM_DUTY_CYCLE_RESET_VALUE; j <= u8power; ++j)
		{
			timer_delay_ms(100);
			for(i = 0; i < PWM_CHMAX; ++i)
			{
				pwm_width_buffer[i] = j;
			}
		}
	}
	else 
	{
		uart_send_char(u8anim);
		uart_newline();
	}
	timer_delay_ms(200);
}

void animation_setallchannels(const bool level)
{
	for(uint8_t i = 0; i < PWM_CHMAX; ++i)
	{
		if(level)
		{
			//pwm_width_buffer[i] = NOCONNECTION_ANIMATION_DEFAULTPWM;
			//pwm_width_buffer[i] = pwm_load_default_dutycycle();
		}
		else
		{
			pwm_width_buffer[i] = 0;
		}
	}
}

uint8_t animation_save_startup_anim(uint8_t u8data)
{
	uint8_t u8data_check;
	//u8data = u8data - 0x30;  //conversion from ASCII
	if( (u8data>=ANIMATION_SUA_NONE) && (u8data<=ANIMATION_SUA_SMOOTH) )
	{
		eeprom_write_byte(EEL_STARTUP_ANIMATION, u8data);
		u8data_check = 1;
	}
	else
	{
		u8data_check = 0;
	}
	return u8data_check;
}

uint8_t animation_save_no_netw_anim(uint8_t u8data)
{
	uint8_t u8data_check;	
	//u8data = u8data - 0x30;  //conversion from ASCII
	if((u8data>= ANIMATION_NONET_NONE)&&(u8data<=ANIMATION_NONET_XDIM))
	{
		eeprom_write_byte(EEL_NO_CONN_NOTIFICATION, u8data);
		u8data_check = 1;
	}
	else
	{
		u8data_check = 0;
	}
	return u8data_check;
}

uint8_t animation_save_no_netw_power(uint8_t u8data)
{
	uint8_t u8data_check;
	if((u8data>=PWM_DUTY_CYCLE_RESET_VALUE)&&(u8data<=PWM_DUTY_MAX_VALUE))
	{
		eeprom_write_byte(EEL_NO_CONN_NOTIFICATION, u8data);
		u8data_check = 1;
	}
	else 
	{
		u8data_check = 0;
	}
	return u8data_check;
}

uint8_t animation_load_startup_anim(void)
{
	return (eeprom_read_byte(EEL_STARTUP_ANIMATION));
}
uint8_t animation_load_no_netw_anim(void)
{
	return (eeprom_read_byte(EEL_NO_CONN_NOTIFICATION));
}
uint8_t animation_load_no_netw_power(void)
{
	return (eeprom_read_byte(EEL_NO_CONN_POWER));
}