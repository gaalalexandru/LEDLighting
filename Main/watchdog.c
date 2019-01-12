/*
 * watchdog.c
 *
 * Created: 1/10/2019 12:57:06 PM
 *  Author: alexandru.gaal
 */ 
#include <avr/io.h>
#include <avr/wdt.h>
#include "watchdog.h"

void watchdog_reset(void) {
	wdt_reset();
}

void watchdog_enable(uint8_t u8wdg_timeot) {
	WDTCR = (1<<WDCE);
	WDTCR |= (1<<WDE);
	WDTCR |= (u8wdg_timeot & (0x07));
}

void watchdog_disable(void) {
	watchdog_reset();
	WDTCR = (1<<WDCE) | (1<<WDE);
	/* Turn off WDT */
	WDTCR = 0x00;
}
