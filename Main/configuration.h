/*
 * configuration.h
 *
 * Created: 10/10/2017 10:39:14 PM
 *  Author: Gaal Alexandru
 */ 

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <avr/io.h>
#include <avr/portpins.h>
#include "esp_wifi_handler_defs.h"

/************************************************************************/
/*						   CONTROLLER SELECTION							*/
/************************************************************************/
#if defined (__AVR_ATmega48PB__)  && !defined (__AVR_ATmega8__)
#define ATMEGA48 (1)
#define ATMEGA8	(0)
#elif defined (__AVR_ATmega8__)  && !defined (__AVR_ATmega48PB__)
#define ATMEGA48 (0)
#define ATMEGA8	(1)
#else
#error "Please specify one target controller"
#endif  //controller selection

/************************************************************************/
/*					         CONTROL METHODE SELECTION					*/
/************************************************************************/
#define ESP_TERMINAL_CONTROL		(0) //allows manually to send AT commands to ESP via terminal
#define PWM_TERMINAL_CONTROL		(0)	//allow PWM channels to be manually controlled via serial terminal
#define LIGHTING_WIFI_CONTROL		(1)	//allow pwm channels to be controlled via wifi interface

/************************************************************************/
/*							FUNCTIONALITY ACTIVATION					*/
/************************************************************************/
#define LIGHTING_FUNCTION_ACTIVE	(1)	//enable timer0 and pwm channel outputs
#define STATUS_LED_ACTIVE			(1)	//enable timer1 and status LED init & toggle
#define DEBUGPIN_ACTIVE				(0)	//enable free pin to be used for debugging / measurements
#define TERMINAL_DEBUG_ACTIVE		(0) //enable the print of various information to terminal
#define STARTUP_ANIMATION_ACTIVE	(1)	//activate startup animation
#define NONET_ANIMATION_ACTIVE		(1)	//activate no network connection animation

/************************************************************************/
/*							UART CONFIGURATIONS							*/
/************************************************************************/
#define FOSC 4000000// Clock Speed
#if PWM_TERMINAL_CONTROL
	#define BAUD 9600
#else
	#define BAUD 38400
#endif
#define SET_U2X (1)
#define DONT_USE_ISR_UART (0)
#if SET_U2X
	#define MYUBRR ((FOSC/(8*BAUD))-1)
#else
	#define MYUBRR ((FOSC/(16*BAUD))-1)
#endif

/************************************************************************/
/*							PWM CONFIGURATIONS							*/
/************************************************************************/
#define PWM_CONFIG_CHMAX 12 //maximum number of pwm channels
#define PWM_CONFIG_DUTY_MAX_VALUE 0x44  //number of duty cycle adjustments 
//0x30 = 0%, 0x31 = 5%, 0x32 = 10%, 0x3A = 50%, 0x3B = 55%
//0x3C = 60%, 0x3D = 65%, 0x3F = 75%, 0x40 = 80%, 0x43 = 95%, 0x44 = 100%
#define PWM_CONFIG_DUTY_CYCLE_RESET_VALUE 0x30  //start value of duty cycle counter
#define PWM_SYM_ALL_CH (12)  //ID to controll all channels
#define PWM_SYM_HALF1_CH (13)  //ID to control 1st half of channels 0 - 5
#define PWM_SYM_HALF2_CH (14)  //ID to control 2nd half of channels 6 - 11

/************************************************************************/
/*						WIFI & ESP CONFIGURATIONS						*/
/************************************************************************/				
// Pin mapping for ESP8266 wifi module reset (RST_ESP) and enable (CH_PD)
#if ATMEGA48
// RST_ESP mapped to MOSI programing pin PE2
// CH_PD mapped MISO programing pin PE3
#define ESP_RST_PORT	PORTE
#define ESP_RST_DDR		DDRE
#define ESP_RST_PIN		PIN2
#define ESP_ENABLE_PORT	PORTE
#define ESP_ENABLE_DDR	DDRE
#define ESP_ENABLE_PIN	PIN3
#elif ATMEGA8
// RST_ESP mapped to MOSI programing pin PB3
// CH_PD mapped MISO programing pin PB4
#define ESP_RST_PORT	PORTB
#define ESP_RST_DDR		DDRB
#define ESP_RST_PIN		PIN3
#define ESP_ENABLE_PORT	PORTB
#define ESP_ENABLE_DDR	DDRB
#define ESP_ENABLE_PIN	PIN4
#endif

//ESP & StateMachine functionality configuration
/* ESP init delay configuration:
 * Sometimes, on some networks it takes a longer time to get the IP,
 * for this reason in state HW_INIT it's possible to incorrectly detect 
 * no network connection. 
 * To avoid this, use a slightly longer init delay.
 * Tests with different init delay values:
 * 10s => works ok on ALL tested networks
 * 8s => works ok on SOME of tested networks
 * 6s => works ok on SOME of tested networks
 */
#define ESP_CONFIG_INIT_DELAY				(12000) //value is ms
#define ESP_CONFIG_TCP_PORT					"1001"
#define ESP_CONFIG_TCP_TIMEOUT				"120"  //seconds before tcp connection is closed
#define ESP_CONFIG_FORCE_WIFI_SETUP			(0)
#define ESP_CONFIG_CONNECTION_MAX_RETRY		(3)
#define ESP_CONFIG_CHECK_AP_OLD_STATE		(0)
#define ESP_CONFIG_CHECK_RUNTIME_CONNECTION	(1)

/************************************************************************/
/*							STATUS LED CONFIGURATIONS					*/
/************************************************************************/
#define STATUS_LED_DDR		DDRD
#define STATUS_LED_PORT		PORTD
#define STATUS_LED_PIN		PIN4
// #define INIT_STATUS_LED		(DDRB |= (1 << PIN2))
// #define TOGGLE_STATUS_LED	(PORTB ^= (1 << PIN2))

/************************************************************************/
/*						ANIMATION CONFIGURATIONS						*/
/************************************************************************/
#define ANIMATION_SYM_SUA_NONE		(0x30)
#define ANIMATION_SYM_SUA_CIRCLE	(0x31)
#define ANIMATION_SYM_SUA_SMOOTH	(0x32)
#define ANIMATION_CONFIG_SUA_SPEED	(70)  //smaller value = faster

#define ANIMATION_SYM_NONET_NONE	(0x30)
#define ANIMATION_SYM_NONET_BLINK	(0x31)
#define ANIMATION_SYM_NONET_XDIM	(0x32)  //LEDs dimmed to X%
#define ANIMATION_CONFIG_NONET_BLINK_SPEED	(10)  //smaller value = faster
#define ANIMATION_CONFIG_NONET_BLINK_POWER	(PWM_CONFIG_DUTY_CYCLE_RESET_VALUE+1)	// set noconnection animation pwm target

/************************************************************************/
/*							EEPROM CONFIG & LAYOUT						*/
/************************************************************************/
#define EEL_ADDR_FIRST_START			(0X0000)  //0=Yes, 1=No
#define EEL_ADDR_STARTUP_ANIMATION		(0x0001)  //30=None, 31=circle, 32=smooth
#define EEL_ADDR_DEFAULT_POWER			(0x0002)  //0x30=0%, 0x44=100%
#define EEL_ADDR_NO_CONN_NOTIFICATION	(0x0003)  //30=None, 31=blink, 32=XX% power
#define EEL_ADDR_NO_CONN_POWER			(0x0004)  //0x30=0%, 0x44=100%
#define EEL_ADDR_DEVICE_ID				(0x0005)  //0x41 -> 0x5A
#define EEL_ADDR_AP_ALWAYS_ON			(0x0006)  //0x30 Not always on, 0x31 Always on
#define EEL_ADDR_ESP_AUTOCONNECT		(0x0007)  //0x30 ESP Autoconnect Off, 0x31 Autoconnect On
#define EEL_ADDR_RESET_COUNT			(0x0008)  //holds the reset count

#define EEL_ADDR_WIFI_CREDENTIALS_LENGTH	(0x000A) //address to store length of wifi credentials string (ssid & password) 
#define EEL_ADDR_WIFI_CREDENTIALS_START		(0x000B) //start address to store wifi credentials
#define EEL_ADDR_WIFI_CREDENTIALS_END		((0x000B)+40)  //end address to store wifi credentials (start + 40 bytes)

#define EEPROM_INITIALIZED		(0x49)  //'I' character, meaning EEPROM is initialized with default values.
#define EEL_FACTORY_SUA			(/*ANIMATION_SYM_SUA_SMOOTH*/ANIMATION_SYM_SUA_CIRCLE)
#define EEL_FACTORY_POWER		(PWM_CONFIG_DUTY_MAX_VALUE)
#define EEL_FACTORY_NNN			(/*ANIMATION_SYM_NONET_NONE*/ANIMATION_SYM_NONET_BLINK)
#define EEL_FACTORY_NNN_PWR		(PWM_CONFIG_DUTY_CYCLE_RESET_VALUE+1)
#define EEL_FACTORY_ID			(EEPROM_INVALID_ID)
#define EEL_FACTORY_AP_ON		(ESP_SYM_AP_ALWAYS_ON)
#define EEL_FACTORY_AUTOCONNECT	(ESP_SYM_AUTOCONNECT_ON)

#define EEPROM_INVALID_ID	(0)
#define EEPROM_MIN_ID		(0x41)
#define EEPROM_MAX_ID		(0x5A)

#define EEPROM_CONFIG_CLEAR_CREDENTIALS_STORAGE	(0)
#define EEPROM_CONFIG_SIZE (0x200)  //512 byte

/************************************************************************/
/*						RESET & WATCHDOG CONFIGURATIONS					*/
/************************************************************************/
#define RESET_CONFIG_CHECK_START_TIME	(100)
#define RESET_CONFIG_CHECK_END_TIME (1500)
#define RESET_SYM_DO_RESET	(0x52) //'R'
#define RESET_SYM_NO_RESET	(0x4E) //'N'

#define WATCHDOG_SYM_TIMEOUT_15MS   (0)
#define WATCHDOG_SYM_TIMEOUT_30MS   (1)
#define WATCHDOG_SYM_TIMEOUT_60MS   (2)
#define WATCHDOG_SYM_TIMEOUT_120MS  (3)
#define WATCHDOG_SYM_TIMEOUT_250MS  (4)
#define WATCHDOG_SYM_TIMEOUT_500MS  (5)
#define WATCHDOG_SYM_TIMEOUT_1S     (6)
#define WATCHDOG_SYM_TIMEOUT_2S     (7)

#define WATCHDOG_CONFIG_TIMEOUT		(WATCHDOG_SYM_TIMEOUT_2S)

#endif /* CONFIGURATION_H_ */