/*
 * esp_wifi_handler.h
 *
 * Created: 10/17/2017 10:15:46 PM
 *  Author: Gaal Alexandru
 */ 

#ifndef WIFI_HANDLER_H_
#define WIFI_HANDLER_H_

uint8_t esp_init_hw(uint16_t u16init_delay);
void esp_state_machine(void);

#endif /* WIFI_HANDLER_H_ */