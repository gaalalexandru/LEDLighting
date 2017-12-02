/*
 * status_led.h
 *
 * Created: 12/2/2017 4:56:24 PM
 *  Author: Gaal Alexandru
 */ 


#ifndef STATUS_LED_H_
#define STATUS_LED_H_

#define STATUS_LED_INTERVALS (15)  //number of 100 ms intervals to control status led

typedef enum
{
	off = 0,
	wait_for_ip = 1,
	connected_to_ap = (STATUS_LED_INTERVALS / 2),
	error_indication = (STATUS_LED_INTERVALS - 2),
	on = (STATUS_LED_INTERVALS - 1)
} status_led_mode_t;

void status_led_init(void);
void status_led_update(void);

#endif /* STATUS_LED_H_ */