/*
 * watchdog.c
 *
 * Created: 1/10/2019 12:57:06 PM
 *  Author: alexandru.gaal
 */ 

#include <avr/wdt.h>

#if 0
_WDR();

WDTCR |= (1<<WDCE) | (1<<WDE) | (1<<WDP2) | (1<<WDP1) | (1<<WDP0) ;

wdt_enable(WDTO_2S);

#endif