/*
 * pwm_handler.c
 *
 * Created: 10/10/2017 10:30:14 PM
 *  Author: Gaal Alexandru, Bogdan Rat
 */ 
#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/portpins.h>

#include "eeprom_handler.h"
#include "pwm_handler.h"
#include "configuration.h"
#include "animation_handler.h"

//macros to set pin assigned as pwm channels
//if layout changes update the macros also
#define PWM_SET_CH0(x)	set_port_d(7,x)
#define PWM_SET_CH1(x)	set_port_b(0,x)
#define PWM_SET_CH2(x)	set_port_d(6,x)
#define PWM_SET_CH3(x)	set_port_d(5,x)
#define PWM_SET_CH4(x)	set_port_b(2,x)
#define PWM_SET_CH5(x)	set_port_b(1,x)
#define PWM_SET_CH6(x)	set_port_c(1,x)
#define PWM_SET_CH7(x)	set_port_c(0,x)
#define PWM_SET_CH8(x)	set_port_c(2,x)
#define PWM_SET_CH9(x)	set_port_c(3,x)
#define PWM_SET_CH10(x)	set_port_c(4,x)
#define PWM_SET_CH11(x)	set_port_c(5,x)

//global buffers
uint8_t pwm_width[PWM_CONFIG_CHMAX];
volatile uint8_t pwm_width_buffer[PWM_CONFIG_CHMAX];

//Set PORT level true = high, false = low
//pin = pin number to be set / cleared
static inline void set_port_b(const uint8_t pin, const bool level)
{
	if (level)
	{
		PORTB |= (1 << pin);
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

static inline void set_port_c(const uint8_t pin, const bool level)
{
	if (level)
	{
		PORTC |= (1 << pin);
	}
	else
	{
		PORTC &= ~(1 << pin);
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
	PORTC = (level << PIN0) | (level << PIN1) | (level << PIN2) | (level << PIN3) | (level << PIN4) | (level << PIN5);
}

uint8_t pwm_save_default_dutycycle(uint8_t u8duty)
{
	uint8_t u8data_check;
	if((u8duty>=PWM_CONFIG_DUTY_CYCLE_RESET_VALUE)&&(u8duty<=PWM_CONFIG_DUTY_MAX_VALUE))
	{
		eeprom_write_byte(EEL_ADDR_DEFAULT_POWER, u8duty);
		u8data_check = 1;
	}
	else
	{
		u8data_check = 0;
	}
	return u8data_check;
}

uint8_t pwm_load_default_dutycycle(void)
{
	return (eeprom_read_byte(EEL_ADDR_DEFAULT_POWER));
}

void pwm_init(void)
{
	uint8_t i = 0; 
	uint8_t pwm = 0;
	uint8_t u8anim = 0;
	
	u8anim = animation_load_startup_anim();
	if(u8anim == ANIMATION_SYM_SUA_NONE)
	//if no startup animation is coded, just simply start LEDs at preset value
	{
		pwm = pwm_load_default_dutycycle();
	} else { //set light off, startup animation will start them
		pwm = PWM_CONFIG_DUTY_CYCLE_RESET_VALUE;
	}
	for (i = 0; i < PWM_CONFIG_CHMAX; i++) // initialize all channels
	{
		pwm_width[i]  = pwm; // set default PWM values
		pwm_width_buffer[i] = pwm; // set default PWM values
		}

	DDRB |= 0x07;  //Set output pin 0, 1, 2 of port B
	DDRD |= 0xE0;  //Set output pin 5, 6, 7 of port D
	DDRC |= 0x3F;  //Set output pin 0, 1, 2, 3, 4, 5 of port C
	pwm_set_all_chanels(false);
}

void pwm_update(void)
{	
	static uint8_t softcount = PWM_CONFIG_DUTY_CYCLE_RESET_VALUE;
	/* increment counter and update
	the pwm_width values only when counter = 0.
	verbose code for speed, do not replace with for...
	last element should equal PWM_CONFIG_CHMAX - 1 */
	//softcount = (softcount + 1) % PWM_DUTY_CYCLE_RESOLUTION;  //slower method
	softcount++;  // faster method with if condition
	if(softcount >= PWM_CONFIG_DUTY_MAX_VALUE)
	{
		softcount = PWM_CONFIG_DUTY_CYCLE_RESET_VALUE;
	}
	if (softcount == PWM_CONFIG_DUTY_CYCLE_RESET_VALUE)
	{
		pwm_width[0] = pwm_width_buffer[0]; 
		pwm_width[1] = pwm_width_buffer[1];
		pwm_width[2] = pwm_width_buffer[2];
		pwm_width[3] = pwm_width_buffer[3];
		pwm_width[4] = pwm_width_buffer[4];
		pwm_width[5] = pwm_width_buffer[5];
		
		pwm_width[6] = pwm_width_buffer[6];
		pwm_width[7] = pwm_width_buffer[7];
		pwm_width[8] = pwm_width_buffer[8];
		pwm_width[9] = pwm_width_buffer[9];
		pwm_width[10] = pwm_width_buffer[10];
		pwm_width[11] = pwm_width_buffer[11];
		
		PWM_SET_CH0((pwm_width[0] > 0x30));
		PWM_SET_CH1((pwm_width[1] > 0x30));
		PWM_SET_CH2((pwm_width[2] > 0x30));
		PWM_SET_CH3((pwm_width[3] > 0x30));
		PWM_SET_CH4((pwm_width[4] > 0x30));
		PWM_SET_CH5((pwm_width[5] > 0x30));
		
		PWM_SET_CH6((pwm_width[6] > 0x30));		
		PWM_SET_CH7((pwm_width[7] > 0x30));
		PWM_SET_CH8((pwm_width[8] > 0x30));
		PWM_SET_CH9((pwm_width[9] > 0x30));
		PWM_SET_CH10((pwm_width[10] > 0x30));
		PWM_SET_CH11((pwm_width[11] > 0x30));		
	}
	else
	{
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
			
		if (pwm_width[6] == softcount)
			PWM_SET_CH6(false);
		if (pwm_width[7] == softcount)
			PWM_SET_CH7(false);
		if (pwm_width[8] == softcount)
			PWM_SET_CH8(false);
		if (pwm_width[9] == softcount)
			PWM_SET_CH9(false);
		if (pwm_width[10] == softcount)
			PWM_SET_CH10(false);
		if (pwm_width[11] == softcount)
			PWM_SET_CH11(false);
	}
}

uint8_t pwm_wifi_update(uint8_t channel_nr, uint8_t duty_cycle)
{
	uint8_t response = 0;
	uint8_t i = 0;
	if((duty_cycle >= PWM_CONFIG_DUTY_CYCLE_RESET_VALUE) && (duty_cycle <= PWM_CONFIG_DUTY_MAX_VALUE) && (channel_nr >= 0)  && (channel_nr <= PWM_SYM_HALF2_CH))  //check for valid inputs
	{
		if(channel_nr < PWM_CONFIG_CHMAX) //decide to control single channels or channel groups
		{
			pwm_width_buffer[channel_nr] = duty_cycle; // update pwm_width buffer
		}
		else if(channel_nr == PWM_SYM_ALL_CH)  //12: all channels 0-11
		{
			for(i=0; i<PWM_CONFIG_CHMAX; i++)
			{
				pwm_width_buffer[i] = duty_cycle;
			}
		}
		else if(channel_nr == PWM_SYM_HALF1_CH)  //13: channel 0-5
		{
			for(i=0; i<PWM_CONFIG_CHMAX/2; i++)
			{
				pwm_width_buffer[i] = duty_cycle;
			}
		}
		else if(channel_nr == PWM_SYM_HALF2_CH)  //14: channel 6-11
		{
			for(i=PWM_CONFIG_CHMAX/2; i<PWM_CONFIG_CHMAX; i++)
			{
				pwm_width_buffer[i] = duty_cycle;
			}
			
		}
		else
		{
			//do nothing
			response = 0;
		}
		response = 1;
	}
	else
	{
		response = 0;
	}
	return response;
}