/*
 * wifi_handler.c
 *
 * Created: 10/17/2017 9:57:41 PM
 *  Author: Gaal Alexandru
 */ 
//Wifi module type: ESP8266

#include <avr/io.h>
#include <avr/portpins.h>

//Pin mapping for wifi module reset (RST_ESP) and enable (CH_PD), pins have to be digital output
//CH_PD: Chip enable. Keep it on high (3.3V) for normal operation
//RST_ESP: Reset. Keep it on high (3.3V) for normal operation. Put it on 0V to reset the chip.
//RST_ESP mapped to MOSI programing pin PB3
//CH_PD mapped MISO programing pin PB4
#define RST_ESP_DIR	DDRB |= (1 << PIN3)
#define	CH_PD_DIR	DDRB |= (1 << PIN4)
#define RST_ESP_SET(x)	PORTB |= ((x) << PIN3)
#define	CH_PD_SET(x)	PORTB |= ((x) << PIN4)  

void wifi_init(void)
{
	RST_ESP_DIR;
	CH_PD_DIR;
	
	RST_ESP_SET(1);
	RST_ESP_SET(0);
	RST_ESP_SET(1);
	
	CH_PD_SET(1);
}