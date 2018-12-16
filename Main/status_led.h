/*
 * status_led.h
 *
 * Created: 12/2/2017 4:56:24 PM
 *  Author: Gaal Alexandru
 */ 


#ifndef STATUS_LED_H_
#define STATUS_LED_H_

#define STATUS_LED_INTERVALS (10)  //number of 100 ms intervals to control status led

typedef enum
{
	off = 0, //1000 ms off
	wait_for_ip = 1, //100 ms on, 900 ms off
	connected_to_ap = (STATUS_LED_INTERVALS / 2),  //500 ms on, 500 ms off
	error_indication = (STATUS_LED_INTERVALS - 2), //900 ms on, 100 ms off
	on = (STATUS_LED_INTERVALS - 1) //1000 ms on
} status_led_mode_t;

void status_led_init(void);
void status_led_update(void);

#endif /* STATUS_LED_H_ */