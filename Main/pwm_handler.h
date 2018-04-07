/*
 * pwm_handler.h
 *
 * Created: 10/10/2017 11:14:33 PM
 *  Author: Ninja
 */ 


#ifndef PWM_HANDLER_H_
#define PWM_HANDLER_H_

/*
enum pwm_channels 
{
	pwm_channel_0 = 0x30;
	pwm_channel_1,
	pwm_channel_2,
	pwm_channel_3,
	pwm_channel_4,
	pwm_channel_5,
	pwm_channel_numbers
	};*/

void pwm_save_default_dutycycle(uint8_t u8duty);
uint8_t pwm_load_default_dutycycle(void);
void pwm_init(void);
void pwm_update(void);


#endif /* PWM_HANDLER_H_ */