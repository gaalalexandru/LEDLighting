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

//#include "configuration.h"

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