/*
 * configuration.h
 *
 * Created: 10/10/2017 10:39:14 PM
 *  Author: Gaal Alexandru
 */ 

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

//uart configurations
#define FOSC 8000000// Clock Speed
#define BAUD 38400 // Old value only for terminal control: 9600
#define SET_U2X (0)

#if SET_U2X
	#define MYUBRR ((FOSC/(8*BAUD))-1)
#else
	#define MYUBRR ((FOSC/(16*BAUD))-1)
#endif

//pwm configurations
#define CHMAX 6 //maximum number of pwm channels
#define PWMDEFAULT 0x01  //default pulse width


#define WIRELESS_CONTROL	(1)	//allow pwm channels to be controlled via wifi interface
#define LIGHTING_FUNCTION	(1)	//enable timer0 and pwm channel outputs
#define STATUS_LED			(1)	//enable timer1 and status LED init & toggle
#define USE_DEBUGPIN		(0)	//enable free pin to be used for debugging / measurements
#define TERMINAL_CONTROL	(0)	//allow PWM channels to be manually controlled via serial terminal
								//message format is #xYY, x = channel number, YY 8bit hex value of pulse width
//wifi configurations
#define WIFI_SSID_PASSWORD		"\"UPC5C34B5E\",\"jsUsje5vd4ue\"\r\n"

//status LED configuration
#define INIT_STATUS_LED		(DDRD |= (1 << PIN4))
#define TOGGLE_STATUS_LED	(PORTD ^= (1 << PIN4))
#define STATUS_LED_ON		(PORTD |= (1 << PIN4))
#define STATUS_LED_OFF		(PORTD &= ~(1 << PIN4))

// #define INIT_STATUS_LED		(DDRB |= (1 << PIN2))
// #define TOGGLE_STATUS_LED	(PORTB ^= (1 << PIN2))

#endif /* CONFIGURATION_H_ */