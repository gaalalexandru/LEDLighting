/*
 * Main.cpp
 *
 * Created: 10/4/2017 10:48:34 AM
 *  Author: dan.filip
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "configuration.h"
#include "usart_handler.h"
#include "timer_handler.h"
#include "pwm_handler.h"
#include "wifi_handler.h"

#define FOSC 8000000// Clock Speed
#define BAUD 115200 // Old value only for terminal control: 9600
#define MYUBRR FOSC/16/BAUD-1

int main(void)
{
	#if TERMINAL_CONTROL
	USART_Init(MYUBRR);
	#endif  //TERMINAL_CONTROL
	
	pwm_init();
	timer0_init();

	sei();  // enable global interrupts
	USART_NewLine();
	USART_OutString("Init Done");

	#if WIRELESS_CONTROL
	wifi_init();
	#endif //WIRELESS_CONTROL
	
    while(1)
    {
		#if TERMINAL_CONTROL
		usart_manual_control();
		#endif  //TERMINAL_CONTROL	
    }
}

#if 0
	uint8_t sreg;
	sreg = SREG;  // Save Global Interrupt Flag
	_CLI();//Disable interrupts
	//atomic operations
	SREG = sreg;  // Restore Global Interrupt Flag
#endif