/*
 * pwm_handler.h
 *
 * Created: 10/10/2017 11:14:33 PM
 *  Author: Ninja
 */ 


#ifndef PWM_HANDLER_H_
#define PWM_HANDLER_H_

uint8_t pwm_save_default_dutycycle(uint8_t u8duty);
uint8_t pwm_load_default_dutycycle(void);

void pwm_init(void);
void pwm_update(void);
uint8_t pwm_wifi_update(uint8_t channel_nr, uint8_t duty_cycle);


#endif /* PWM_HANDLER_H_ */