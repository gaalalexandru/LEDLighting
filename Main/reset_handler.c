/*
 * reset_handler.c
 *
 * Created: 1/7/2019 3:03:08 PM
 *  Author: alexandru.gaal
 */ 
#include "reset_handler.h"
#include "eeprom_handler.h"
#include "configuration.h"

uint8_t reset_check(void) 
{
	uint8_t = u8reset_result = 0;
	uint8_t u8reset1 = eeprom_read_byte(EEL_ADDR_TIMESTAMP_RESET1);
	if(u8reset1 == RESET_SYM_DO_RESET){
		//check reset 2
			//check reset 3
				u8reset_result = RESET_SYM_DO_RESET;
	} else {
		//set byte 1 for reset 1 to no reset
		
		u8reset_result = RESET_SYM_NO_RESET;
	}
	
	#if 0
	if((u8reset1 >= 1) && (u8reset1  <= 3)) {
		uint8_t u8reset2 = eeprom_read_byte(EEL_ADDR_TIMESTAMP_RESET2);
		if((u8reset2 >= 1) && (u8reset2  <= 3)) {
			uint8_t u8reset3 = eeprom_read_byte(EEL_ADDR_TIMESTAMP_RESET3);
			if((u8reset3 >= 1) && (u8reset3  <= 3)) {
				eeprom_read_byte(EEL_ADDR_TIMESTAMP_RESET1,0xFF);
				eeprom_read_byte(EEL_ADDR_TIMESTAMP_RESET2,0xFF);
				eeprom_read_byte(EEL_ADDR_TIMESTAMP_RESET3,0xFF);
				reset_eeprom();
				reset_controller();
			}
		}
	}
	#endif
}

void reset_controller(void) {
	
}

void reset_eeprom(void) {
	
}