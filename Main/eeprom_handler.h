/*
 * eeprom_handler.h
 *
 * Created: 4/7/2018 7:39:13 PM
 *  Author: Gaal Alexandru
 */ 


#ifndef EEPROM_HANDLER_H_
#define EEPROM_HANDLER_H_

#include "eeprom_layout.h"

void eeprom_write_byte(uint16_t u16address, uint8_t u8data);
uint8_t eeprom_read_byte(uint16_t u16address);

uint8_t eeprom_load_id(void);
uint8_t eeprom_save_id(uint8_t u8dev_id);

#endif /* EEPROM_HANDLER_H_ */