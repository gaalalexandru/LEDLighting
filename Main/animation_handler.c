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
#include "errors.h"

#define true	1
#define false	0

extern volatile uint8_t pwm_width_buffer[PWM_CONFIG_CHMAX];
static uint8_t animation_sua_finished = 0;

void animation_play_startup(void)
{
	uint8_t i;
	uint8_t u8anim = 0;
	uint8_t u8power = 0;
	
	u8anim = animation_load_startup_anim();
	
	switch (u8anim)
	{
		case ANIMATION_SYM_SUA_NONE:
			//"0" = no startup animation
		break;
		
		case ANIMATION_SYM_SUA_CIRCLE:
			//circle startup animation
			//"1" = starts one led strip one after another
			u8power = pwm_load_default_dutycycle();
			for(i = 0; i < PWM_CONFIG_CHMAX; ++i)
			{
				timer_delay_ms(ANIMATION_CONFIG_SUA_SPEED);
				pwm_width_buffer[i] = u8power;
			}
		break;
		
		case ANIMATION_SYM_SUA_SMOOTH:
			//smooth startup animation
			//"2" = start all led strips with increasing intensity
			u8power = pwm_load_default_dutycycle();
			for(uint8_t j = PWM_CONFIG_DUTY_CYCLE_RESET_VALUE; j <= u8power; ++j)
			{
				timer_delay_ms(ANIMATION_CONFIG_SUA_SPEED);
				for(i = 0; i < PWM_CONFIG_CHMAX; ++i)
				{
					pwm_width_buffer[i] = j;
				}
			}
		break;
			
		default:
			uart_send_string(ERROR_ANIMATION_UndefinedStartupAnimationCoded);
		break;
	}
	animation_sua_finished = 1;
}

void animation_play_nonetwork(void)
{
	uint8_t i;
	static uint8_t u8anim = 0;
	static uint8_t u8power = 0;
	static uint8_t u8state = 0;
	static uint8_t u8first_start = 1;
	
	if(u8first_start){
		u8anim = animation_load_no_netw_anim();
		u8power = animation_load_no_netw_power();
		u8first_start = 0;
	}
	if(animation_sua_finished)
	{
		switch (u8anim)
		{
			case ANIMATION_SYM_NONET_NONE:
			//"0" = no startup animation
			break;
			
			case ANIMATION_SYM_NONET_BLINK:
			//"1" = slowly blink all leds (use a low PWM duty cycle when ON)
			if (u8state) {
				for(i = 0; i < PWM_CONFIG_CHMAX; ++i) {
					pwm_width_buffer[i] = ANIMATION_CONFIG_NONET_BLINK_POWER;
				}
				} else {
				for(i = 0; i < PWM_CONFIG_CHMAX; ++i) {
					pwm_width_buffer[i] = PWM_CONFIG_DUTY_CYCLE_RESET_VALUE;
				}
			}
			u8state ^= 1;
			break;
			
			case ANIMATION_SYM_NONET_XDIM:
			//"2" = dim all LED to a set value in EEPROM
			for(i = 0; i < PWM_CONFIG_CHMAX; ++i) {
				pwm_width_buffer[i] = u8power;
			}
			break;
			
			default:
			uart_send_string(ERROR_ANIMATION_UndefinedNoNetAnimationCoded);
			break;
		}
	}
}

void animation_stop_nonetwork(void)
{
	uint8_t i;
	uint8_t u8power = 0;
	u8power = pwm_load_default_dutycycle();
	for(i = 0; i < PWM_CONFIG_CHMAX; ++i)
	{
		pwm_width_buffer[i] = u8power;
	}
}

uint8_t animation_save_startup_anim(uint8_t u8data)
{
	uint8_t u8data_check;
	//u8data = u8data - 0x30;  //conversion from ASCII
	if( (u8data>=ANIMATION_SYM_SUA_NONE) && (u8data<=ANIMATION_SYM_SUA_SMOOTH) )
	{
		eeprom_write_byte(EEL_ADDR_STARTUP_ANIMATION, u8data);
		u8data_check = 1;
	} else {
		u8data_check = 0;
	}
	return u8data_check;
}

uint8_t animation_save_no_netw_anim(uint8_t u8data)
{
	uint8_t u8data_check;	
	if((u8data>= ANIMATION_SYM_NONET_NONE)&&(u8data<=ANIMATION_SYM_NONET_XDIM))
	{
		eeprom_write_byte(EEL_ADDR_NO_CONN_NOTIFICATION, u8data);
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
	if((u8data>=PWM_CONFIG_DUTY_CYCLE_RESET_VALUE)&&(u8data<=PWM_CONFIG_DUTY_MAX_VALUE))
	{
		eeprom_write_byte(EEL_ADDR_NO_CONN_POWER, u8data);
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
	return (eeprom_read_byte(EEL_ADDR_STARTUP_ANIMATION));
}
uint8_t animation_load_no_netw_anim(void)
{
	return (eeprom_read_byte(EEL_ADDR_NO_CONN_NOTIFICATION));
}
uint8_t animation_load_no_netw_power(void)
{
	return (eeprom_read_byte(EEL_ADDR_NO_CONN_POWER));
}