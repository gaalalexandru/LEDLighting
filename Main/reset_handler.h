/*
 * reset_handler.h
 *
 * Created: 1/7/2019 3:03:19 PM
 *  Author: alexandru.gaal
 */ 


#ifndef RESET_HANDLER_H_
#define RESET_HANDLER_H_

uint8_t reset_check(void);
void reset_clear(void);
void reset_controller(void);
void reset_eeprom(void);

#endif /* RESET_HANDLER_H_ */