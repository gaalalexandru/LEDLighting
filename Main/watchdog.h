/*
 * watchdog.h
 *
 * Created: 1/10/2019 12:57:17 PM
 *  Author: alexandru.gaal
 */ 

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

void watchdog_reset(void);
void watchdog_enable(uint8_t u8wdg_timeot);
void watchdog_disable(void);

#endif /* WATCHDOG_H_ */