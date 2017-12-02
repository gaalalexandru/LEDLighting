/*
 * status_led.c
 *
 * Created: 12/2/2017 4:55:48 PM
 *  Author: Gaal Alexandru
 */ 

// The status led shows to current state of the wifi connection:
// very short period on, long period off	=> trying to connect to AP
// 50% off, 50% on							=> connected to AP
// long period on, very short period off	=> error occured
#include "status_led.h"
#include "configuration.h"

#define INIT_STATUS_LED		(STATUS_LED_DDR |= (1 << STATUS_LED_PIN))
#define TOGGLE_STATUS_LED	(STATUS_LED_PORT ^= (1 << STATUS_LED_PIN))
#define STATUS_LED_ON		(STATUS_LED_PORT |= (1 << STATUS_LED_PIN))
#define STATUS_LED_OFF		(STATUS_LED_PORT &= ~(1 << STATUS_LED_PIN))

/************************************************************************/
/*                           Global variables                           */
/************************************************************************/
volatile status_led_mode_t status_led_mode = off;

/************************************************************************/
/*                      Status Led Control functions                    */
/************************************************************************/
void status_led_init(void)
{
	INIT_STATUS_LED;
	status_led_mode = off;
}

void status_led_update(void)
{
	static uint8_t interval_counter = 0;

	switch (status_led_mode)
	{
		case off:
			STATUS_LED_OFF;
		break;
		
		case on:
			STATUS_LED_ON;
		break;
		
		default:  // if all the other modes between 1 and 8
			if(interval_counter == 0)  //on 1st interval turn led on
			{
				STATUS_LED_ON;
			}
			else if(interval_counter == status_led_mode) //on mode-th interval turn led off
			{
				STATUS_LED_OFF;
			}
		break;
	}
	
	interval_counter = ((interval_counter + 1) % STATUS_LED_INTERVALS);
}