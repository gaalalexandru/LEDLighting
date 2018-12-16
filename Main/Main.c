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
#include "animation_handler.h"
#include "eeprom_handler.h"

int main(void)
{
	eeprom_init();
	
	cli();  //Disable interrupts

	#if DEBUGPIN_ACTIVE
	DDRC = 0xFF;
	PORTC = 0x00;
	#endif //DEBUGPIN
	
	//global timer init
	timer2_init();
	
	#if STATUS_LED_ACTIVE
	status_led_init();
	timer1_init();
	#endif  //STATUS_LED_ACTIVE
	
	#if ESP_TERMINAL_CONTROL
	//do not initialize UART on ATMEGA
	esp_hw_init(); //turn on ESP pins
	#else
	uart_init(MYUBRR);
	#endif //ESP_TERMINAL_CONTROL
	
	#if LIGHTING_FUNCTION_ACTIVE
	pwm_init();
	timer0_init();
	#endif  //LIGHTING_FUNCTIN
	
	sei();  // enable global interrupts
	
	#if STARTUP_ANIMATION_ACTIVE
	animation_init();
	#endif //STARTUP_ANIMATION_ACTIVE
	
    while(1)
    {
		#if PWM_TERMINAL_CONTROL
		manual_control();
		#endif  //PWM_TERMINAL_CONTROL
		
		#if LIGHTING_WIFI_CONTROL
			#if ESP_TERMINAL_CONTROL
			//do nothing with ESP	
			#else
			esp_state_machine();
			#endif //ESP_TERMINAL_CONTROL
		#endif //LIGHTING_WIFI_CONTROL
    }
}
