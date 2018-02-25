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
#define USE_DEBUGPIN		(1)	//enable free pin to be used for debugging / measurements
#define TERMINAL_CONTROL	(0)	//allow PWM channels to be manually controlled via serial terminal
//message format is #xYY, x = channel number, YY 8bit hex value of pulse width
#define TERMINAL_DEBUG		(0) //enable the print of various information to terminal

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
#define FOSC 4000000// Clock Speed
#define BAUD 38400 // Old value only for terminal control: 9600
#define SET_U2X (1)
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
#define PWM_DUTY_CYCLE_RESOLUTION 20  //number of duty cycle adjustments 
//0x30 = 0%, 0x31 = 5%, 0x32 = 10%, 0x3A = 50%, 0x3B = 55%
//0x3C = 60%, 0x3D = 65%, 0x3F = 75%, 0x40 = 80%, 0x43 = 95%, 0x44 = 100%
#define PWM_DUTY_CYCLE_RESET_VALUE 0x30  //start value of duty cycle counter
//////////////////////////////////////////////////////////////////////////
//						WIFI & ESP CONFIGURATIONS						//
//////////////////////////////////////////////////////////////////////////					
//#define WIFI_SSID_PASSWORD	"\"UPC5C34B5E\",\"jsUsje5vd4ue\"\r\n"		/*232255504335433334423545222C226A7355736A65357664347565220D0A*/
//#define WIFI_SSID_PASSWORD	"\"AndroidAP\",\"stargate\"\r\n"			/*2322416E64726F69644150222C227374617267617465220D0A*/
//#define WIFI_SSID_PASSWORD	"\"ASUS_X008D\",\"86c423b622c8\"\r\n"		/*2322415355535F5830303844222C22383663343233623632326338220D0A*/
//#define WIFI_SSID_PASSWORD	"\"MyASUS\",\"Zuzuk1man\"\r\n"				/*23224D7941535553222C225A757A756B316D616E220D0A*/
//#define WIFI_SSID_PASSWORD	"\"FELINVEST\",\"1234qwe$\"\r\n"			/*232246454C494E56455354222C223132333471776524220D0A*/
//#define WIFI_SSID_PASSWORD	"\"BogdanMobile\",\"bogdan123\"\r\n" 		/*2322426F6764616E4D6F62696C65222C22626F6764616E313233220D0A*/
#define WIFI_SSID_PASSWORD		"Blank"

#define ESP_CFG_DEV_PORT	"1003"  //port of device that send the config to ESP
#define ESP_AP_PORT			"1002"  //port of ESP Access Point TCP Server
#define ESP_STA_PORT		"1001"  //port of ESP Station TCP Server
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