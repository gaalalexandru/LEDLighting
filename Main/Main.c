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
#include "esp_wifi_handler.h"
#include "manual_control.h"
#include "status_led.h"

int main(void)
{
	cli();  //Disable interrupts*/

	#if USE_DEBUGPIN
	DDRC = 0xFF;
	PORTC = 0x00;
	#endif //DEBUGPIN
	
	//global timer init
	timer2_init();
	
	#if STATUS_LED
	status_led_init();
	timer1_init();
	#endif  //STATUS_LED
	
	#if ESP_MANUAL_CONTROL
	//do not initialize UART on ATMEGA8
	#else
	uart_init(MYUBRR);
	#endif //ESP_MANUAL_CONTROL
	
	#if LIGHTING_FUNCTION
	pwm_init();
	timer0_init();
	#endif  //LIGHTING_FUNCTIN
	
	sei();  // enable global interrupts
	
	#if WIRELESS_CONTROL
	esp_init();
	esp_check_current_setup();
	#endif //WIRELESS_CONTROL
	
    while(1)
    {
		#if TERMINAL_CONTROL
		manual_control();
		#endif  //TERMINAL_CONTROL
		
		#if WIRELESS_CONTROL
			#if ESP_MANUAL_CONTROL
			//do nothing with ESP	
			#else
			esp_state_machine();
			#endif //ESP_MANUAL_CONTROL
		#endif //WIRELESS_CONTROL
		
		#if 0
		timer_delay_ms(1000);
		PORTC ^= 0xFF;
		uart_send_udec(timer_ms());
		uart_newline();
		#endif
    }
}

#if 0
	uint8_t sreg;
	sreg = SREG;  // Save Global Interrupt Flag
	cli();//Disable interrupts
	//atomic operations
	SREG = sreg;  // Restore Global Interrupt Flag
#endif