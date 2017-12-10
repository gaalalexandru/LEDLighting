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

//////////////////////////////////////////////////////////////////////////
//							FUNCTIONALITY ACTIVATION					//
//////////////////////////////////////////////////////////////////////////
#define ESP_MANUAL_CONTROL	(0) //allows manually to send AT commands to ESP via terminal
#define WIRELESS_CONTROL	(1)	//allow pwm channels to be controlled via wifi interface
#define LIGHTING_FUNCTION	(1)	//enable timer0 and pwm channel outputs
#define STATUS_LED			(1)	//enable timer1 and status LED init & toggle
#define USE_DEBUGPIN		(0)	//enable free pin to be used for debugging / measurements
#define TERMINAL_CONTROL	(0)	//allow PWM channels to be manually controlled via serial terminal
//message format is #xYY, x = channel number, YY 8bit hex value of pulse width
#define TERMINAL_DEBUG		(1) //enable the print of various information to terminal

//////////////////////////////////////////////////////////////////////////
//							CONTROLLER SELECTION						//
//////////////////////////////////////////////////////////////////////////
#if defined (__AVR_ATmega48PB__)
 #define ATMEGA48 (1)
 #define ATMEGA8	(0)
#elif defined (__AVR_ATmega8__)
 #define ATMEGA48 (0)
 #define ATMEGA8	(1)
# endif

//////////////////////////////////////////////////////////////////////////
//							UART CONFIGURATIONS							//
//////////////////////////////////////////////////////////////////////////
#define FOSC 8000000// Clock Speed
#define BAUD 38400 // Old value only for terminal control: 9600
#define SET_U2X (0)
#define DONT_USE_ISR_UART (0)
#if SET_U2X
	#define MYUBRR ((FOSC/(8*BAUD))-1)
#else
	#define MYUBRR ((FOSC/(16*BAUD))-1)
#endif

//////////////////////////////////////////////////////////////////////////
//							PWM CONFIGURATIONS							//
//////////////////////////////////////////////////////////////////////////
#define CHMAX 6 //maximum number of pwm channels
#define PWMDEFAULT 0x00  //default pulse width

//////////////////////////////////////////////////////////////////////////
//						WIFI & ESP CONFIGURATIONS						//
//////////////////////////////////////////////////////////////////////////					
#define WIFI_SSID_PASSWORD		"\"UPC5C34B5E\",\"jsUsje5vd4ue\"\r\n"
//#define WIFI_SSID_PASSWORD	"\"ASUS_X008D\",\"86c423b622c8\"\r\n"
//#define WIFI_SSID_PASSWORD	"\"My ASUS\",\"Zuzuk1man\"\r\n"
#define ESP_AP_PORT		"1002"
#define ESP_STA_PORT	"1001"
#define ESP_SSID_MAX_LENGTH	20
#define ESP_PASS_MAX_LENGTH	20
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

//////////////////////////////////////////////////////////////////////////
//							STATUS LED CONFIGURATIONS					//
//////////////////////////////////////////////////////////////////////////
#define STATUS_LED_DDR		DDRD
#define STATUS_LED_PORT		PORTD
#define STATUS_LED_PIN		PIN4
// #define INIT_STATUS_LED		(DDRB |= (1 << PIN2))
// #define TOGGLE_STATUS_LED	(PORTB ^= (1 << PIN2))

#endif /* CONFIGURATION_H_ */