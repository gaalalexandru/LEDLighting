/*
 * reset_handler.c
 *
 * Created: 1/7/2019 3:03:08 PM
 *  Author: alexandru.gaal
 */ 
#include <stdint.h>
#include <avr/interrupt.h>
#include "configuration.h"
#include "reset_handler.h"
#include "eeprom_handler.h"
#include "watchdog.h"
#include <avr/wdt.h>

uint8_t reset_check(void) 
{
	uint8_t u8reset_count = eeprom_read_byte(EEL_ADDR_RESET_COUNT);
	uint8_t u8reset_result = 0;
	
	switch(u8reset_count) {
		case 0:
		case 1:
		case 2: 
			u8reset_count++;
			eeprom_write_byte(EEL_ADDR_RESET_COUNT, u8reset_count);
			u8reset_result = RESET_SYM_NO_RESET;
			
			break;
		case 3:
			eeprom_write_byte(EEL_ADDR_RESET_COUNT, 0);
			u8reset_result = RESET_SYM_DO_RESET;
		break;
		
		default:
			u8reset_result = RESET_SYM_NO_RESET;
		break;
	}
	return u8reset_result;
}

void reset_clear(void) {
	eeprom_write_byte(EEL_ADDR_RESET_COUNT, 0);
}

void reset_eeprom(void) {
	eeprom_write_byte(EEL_ADDR_FIRST_START,0x00);  
	eeprom_init();
}

void reset_controller(void) {
	cli();  //Disable interrupts
	//do not use own watchdog use library instead
	//watchdog_enable(WATCHDOG_CONFIG_TIMEOUT);
	//wdt_enable(WDTO_2S);
	while(1){/*infinite loop to trigger wdg timeout and cpu reset*/}	
}