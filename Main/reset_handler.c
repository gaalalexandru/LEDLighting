/*
 * reset_handler.c
 *
 * Created: 1/7/2019 3:03:08 PM
 *  Author: alexandru.gaal
 */ 
#include <stdint.h>
#include "reset_handler.h"
#include "eeprom_handler.h"
#include "uart_handler.h"
#include "configuration.h"

uint8_t reset_check(void) 
{
	uint8_t u8reset_count = eeprom_read_byte(EEL_ADDR_RESET_COUNT);
	
	
	switch(u8reset_count){
		case 0:
		case 1:
		case 2: 
			u8reset_count++;
			eeprom_write_byte(EEL_ADDR_RESET_COUNT, u8reset_count);
			
			break;
		case 3:
			eeprom_write_byte(EEL_ADDR_RESET_COUNT, 0);
			reset_eeprom();
			reset_controller();
	}
	return 1;
}

void reset_clear(void) {
	eeprom_write_byte(EEL_ADDR_RESET_COUNT, 0);
	//uart_send_string("CLR RST"); uart_newline();
	//STATUS_LED_PORT = (1<<STATUS_LED_PIN);
}

void reset_controller(void) {
	//uart_send_string("MCU RST"); uart_newline();
}

void reset_eeprom(void) {
	//uart_send_string("EEP RST"); uart_newline();
}
