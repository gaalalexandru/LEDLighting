/*
 * configuration.h
 *
 * Created: 10/10/2017 10:39:14 PM
 *  Author: Gaal Alexandru
 */ 

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

//usart configurations
#define FOSC 8000000// Clock Speed
#define F_CPU FOSC
#define BAUD 115200 // Old value only for terminal control: 9600
#define MYUBRR FOSC/16/BAUD-1

//pwm configurations
#define CHMAX 6 //maximum number of pwm channels
#define PWMDEFAULT 0x00  //default pulse width

//allow PWM channels to be manually controlled via serial terminal
//message format is #xYY, x = channel number, YY 8bit hex value of pulse width
#define TERMINAL_CONTROL (0)  

//allow pwm channels to be controlled via wifi interface
#define WIRELESS_CONTROL (1)

//allow the control and output on pwm channels
#define LIGHTING_FUNCTIN (1)
#endif /* CONFIGURATION_H_ */