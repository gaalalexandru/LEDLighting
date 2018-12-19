/*
 * eeprom_handler.c
 *
 * Created: 4/7/2018 7:38:47 PM
 *  Author: Gaal Alexandru
 */ 
/*#include <stdbool.h>*/
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
/*#include <avr/portpins.h>*/

#include "eeprom_handler.h"
#include "configuration.h"

void eeprom_init(void)
{
	if(eeprom_read_byte(EEL_FIRST_START) != EEPROM_INITIALIZED)
	//check if the SW is started up for the first time after programing
	//if yes, the eeprom is initialized with default values
	{
		eeprom_write_byte(EEL_FIRST_START, EEPROM_INITIALIZED);
		eeprom_write_byte(EEL_STARTUP_ANIMATION, EEL_FACTORY_SUA);
		eeprom_write_byte(EEL_DEFAULT_POWER, EEL_FACTORY_POWER);
		eeprom_write_byte(EEL_NO_CONN_NOTIFICATION, EEL_FACTORY_NNN);
		eeprom_write_byte(EEL_NO_CONN_POWER, EEL_FACTORY_NNN_PWR);
		eeprom_write_byte(EEL_DEVICE_ID, EEL_FACTORY_ID);
		eeprom_write_byte(EEL_AP_ALWAYS_ON, EEL_FACTORY_AP_ON);
	}
}

void eeprom_write_byte(uint16_t u16address, uint8_t u8data)
{
	cli();  //Disable global interrupts*/
	while(EECR & (1<<EEWE))
	{
		/* Wait for completion of previous write */
	}
	/* Set up address and data registers */
	EEAR = u16address;
	EEDR = u8data;
	/* Write logical one to EEMWE - Master Write Enable*/
	EECR |= (1<<EEMWE);
	/* Start eeprom write by setting EEWE - Write Enable*/
	EECR |= (1<<EEWE);
	sei(); //Enable global interrupts
}

uint8_t eeprom_read_byte(uint16_t u16address)
{
	cli();  //Disable global interrupts*/
	while(EECR & (1<<EEWE))
	{
		/* Wait for completion of previous write */
	}
	/* Set up address register */
	EEAR = u16address;
	/* Start eeprom read by writing EERE - Read Enable*/
	EECR |= (1<<EERE);
	/* Return data from data register */
	sei(); //Enable global interrupts
	return EEDR;
}

uint8_t eeprom_load_id(void)
{
	uint8_t u8dev_id = 0;
	u8dev_id = eeprom_read_byte(EEL_DEVICE_ID);
	if((u8dev_id >= EEPROM_MIN_ID) && (u8dev_id <= EEPROM_MAX_ID))
	{
		//do nothing 
	}
	else 
	{
		u8dev_id = 0;
	}
	return u8dev_id;
}

uint8_t eeprom_save_id(uint8_t u8dev_id)
{
	uint8_t u8response = 0;

	if((u8dev_id >= EEPROM_MIN_ID) && (u8dev_id <= EEPROM_MAX_ID))
	{
		eeprom_write_byte(EEL_DEVICE_ID, u8dev_id);
		u8response = 1;
	}
	else
	{
		u8response = 0;
	}
	return u8response;
}

int8_t eeprom_save_wifi_credentials(char *pcwifi_credentials) 
{
	uint8_t i8wifi_credentials_length = 0;
	uint16_t u16wifi_credentials_address = EEL_WIFI_CREDENTIALS_START;
	
	#if EEPROM_CONFIG_CLEAR_CREDENTIALS_STORAGE
	for (u16wifi_credentials_address = EEL_WIFI_CREDENTIALS_START; \
		u16wifi_credentials_address < EEL_WIFI_CREDENTIALS_END; \
		u16wifi_credentials_address++) {
		eeprom_write_byte(u16wifi_credentials_address,0);
	}
	u16wifi_credentials_address = EEL_WIFI_CREDENTIALS_START;
	#endif //EEPROM_CONFIG_CLEAR_CREDENTIALS_STORAGE
	do
	{
		eeprom_write_byte(u16wifi_credentials_address,*pcwifi_credentials);
		pcwifi_credentials++;
		i8wifi_credentials_length++;
		u16wifi_credentials_address++;
	} while((*pcwifi_credentials) && (u16wifi_credentials_address < EEL_WIFI_CREDENTIALS_END));
	/* End of the allocated 40 bytes for storing credentials is reached 
	 * but there are more data in the string to store, set return value to -1
	 */
	if ((u16wifi_credentials_address == EEL_WIFI_CREDENTIALS_END) && (*pcwifi_credentials))
	{
		i8wifi_credentials_length = -1;
	}
	//in case of a valid length, save it to address EEL_WIFI_CREDENTIALS_LENGTH for later use
	if (i8wifi_credentials_length > 0)
	{
		eeprom_write_byte(EEL_WIFI_CREDENTIALS_LENGTH,(uint8_t)i8wifi_credentials_length);
	}
	return i8wifi_credentials_length;
}

int8_t eeprom_load_wifi_credentials(char *pcwifi_credentials)
{
	uint8_t u8wifi_credentials_length = 0;
	int8_t i8response = 0;
	u8wifi_credentials_length = eeprom_read_byte(EEL_WIFI_CREDENTIALS_LENGTH);
	i8response = u8wifi_credentials_length;
	uint16_t u16wifi_credentials_address = EEL_WIFI_CREDENTIALS_START;
	while((u8wifi_credentials_length) && (u16wifi_credentials_address <= EEL_WIFI_CREDENTIALS_END));
	{
		*pcwifi_credentials = eeprom_read_byte(EEL_WIFI_CREDENTIALS_START);
		u8wifi_credentials_length--;
		pcwifi_credentials++;
	}
	//if remaining length != 0 and end address is reached, set return value to -1
	if ((u8wifi_credentials_length) && (u16wifi_credentials_address = EEL_WIFI_CREDENTIALS_END))
	{
		i8response = -1;
	}
	return i8response;
}