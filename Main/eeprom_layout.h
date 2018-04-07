/*
 * eeprom_layout.h
 *
 * Created: 4/7/2018 11:27:34 PM
 *  Author: Gaal Alexandru
 * Current header file contains the address definitions for various settings save in EEPROM
 */ 


#ifndef EEPROM_LAYOUT_H_
#define EEPROM_LAYOUT_H_

#define EEL_FIRST_START			(0X0000)	//0=Yes, 1=No
#define EEL_STARTUP_ANIMATION	(0x0001)	//0=None, 1=circle, 2=smooth
#define EEL_DEFAULT_POWER		(0x0002)	//0x30=0%, 0x44=100%
#define EEL_NO_NET_NOTIFICATION	(0x0003)	//0=None, 1=blink, 2=XX% power
#define EEL_NO_NET_POWER		(0x0004)	//0x30=0%, 0x44=100%

#endif /* EEPROM_LAYOUT_H_ */