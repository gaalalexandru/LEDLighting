/*
 * animation_handler.h
 *
 * Created: 3/7/2018 10:49:58 AM
 *  Author: Bogdan Rad, Alexandru Gaal
 */ 


#ifndef ANIMATION_HANDLER_H_
#define ANIMATION_HANDLER_H_

#include <stdbool.h>

void animation_play(void);
void animation_setallchannels(const bool level);

uint8_t animation_save_startup_anim(uint8_t u8data);
uint8_t animation_save_no_netw_anim(uint8_t u8data);
uint8_t animation_save_no_netw_power(uint8_t u8data);

uint8_t animation_load_startup_anim(void);
uint8_t animation_load_no_netw_anim(void);
uint8_t animation_load_no_netw_power(void);


#endif /* ANIMATION_HANDLER_H_ */