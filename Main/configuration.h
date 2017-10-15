/*
 * configuration.h
 *
 * Created: 10/10/2017 10:39:14 PM
 *  Author: Alexandru Gaal
 */ 

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

//pwm configurations
#define CHMAX 6 //maximum number of pwm channels
#define PWMDEFAULT 0x00  //default pulse width

//allow PWM channels to be manually controlled via serial terminal
//message format is #xYY, x = channel number, YY 8bit hex value of pulse width
#define TERMINAL_CONTROL (1)  

//allow pwm channels to be manually controlled via wifi interface
#define WIRELESS_CONTROL (0)

#endif /* CONFIGURATION_H_ */