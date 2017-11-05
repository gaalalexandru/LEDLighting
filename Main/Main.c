/*
 * Main.cpp
 *
 * Created: 10/4/2017 10:48:34 AM
 *  Author: dan.filip
 */ 
//#include <avr/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "configuration.h"
#include "uart_handler.h"
#include "timer_handler.h"
#include "pwm_handler.h"
#include "wifi_handler.h"
#include "manual_control.h"

int main(void)
{
	//uart_init(MYUBRR);
	sei();  // enable global interrupts
	uart_send_string("UART init done");
#ifdef DEBUGPIN
	DDRC = 0xFF;
	PORTC = 0x00;
#endif //DEBUGPIN
	timer2_init();
	timer1_init();
	INIT_STATUS_LED;

	#if 0//LIGHTING_FUNCTIN
	pwm_init();
	timer0_init();
	#endif
	
	sei();  // enable global interrupts
	
	#if WIRELESS_CONTROL
	wifi_init();
	#endif //WIRELESS_CONTROL
    while(1)
    {
		#if TERMINAL_CONTROL
		manual_control();
		#endif  //TERMINAL_CONTROL
		esp_state_machine();
		/*timer_delay_ms(1000);
		TOGGLE_STATUS_LED;
		uart_send_udec(timer_ms());
		uart_newline();*/
    }
}

#if 0
	uint8_t sreg;
	sreg = SREG;  // Save Global Interrupt Flag
	cli();//Disable interrupts
	//atomic operations
	SREG = sreg;  // Restore Global Interrupt Flag
#endif