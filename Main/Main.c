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
#include "reset_handler.h"
#include "animation_handler.h"

#define INIT_STATUS_LED		(STATUS_LED_DDR |= (1 << STATUS_LED_PIN))
#define TOGGLE_STATUS_LED	(STATUS_LED_PORT ^= (1 << STATUS_LED_PIN))
#define STATUS_LED_ON		(STATUS_LED_PORT |= (1 << STATUS_LED_PIN))
#define STATUS_LED_OFF		(STATUS_LED_PORT &= ~(1 << STATUS_LED_PIN))

#define RST_ESP_DIR	ESP_RST_DDR |= (1 << ESP_RST_PIN)
#define	CH_PD_DIR	ESP_ENABLE_DDR |= (1 << ESP_ENABLE_PIN)
#define RST_ESP_SET(x)	((x) ? (ESP_RST_PORT |= (1 << ESP_RST_PIN)) : (ESP_RST_PORT &= ~(1 << ESP_RST_PIN)))
#define	CH_PD_SET(x)	((x) ? (ESP_ENABLE_PORT |= (1 << ESP_ENABLE_PIN)) : (ESP_ENABLE_PORT &= ~(1 << ESP_ENABLE_PIN)))


typedef enum {
	reset_check_not_done = 0, //1000 ms off
	reset_checking = 1, //100 ms on, 900 ms off
	reset_check_done = 2,  //500 ms on, 500 ms off
	reset_check_nr_of_states
} reset_check_state_t;

extern volatile uint32_t timer_system_ms;
extern volatile bool esp_power_up;

int main(void) {
	//reset_check_state_t reset_check_state = reset_check_not_done;
	uint8_t reset_check_state = reset_check_not_done;

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
	
	#if ESP_TERMINAL_CONTROL //do not initialize UART on ATMEGA
	esp_init_hw(); //turn on ESP pins
	#else
	uart_init(MYUBRR);
	#endif //ESP_TERMINAL_CONTROL
	
	#if LIGHTING_FUNCTION_ACTIVE
	pwm_init();
	timer0_init();
	#endif  //LIGHTING_FUNCTIN
	
	sei();  // enable global interrupts
	
	INIT_STATUS_LED;
	
	if(esp_power_up) {
		//do this only once
		RST_ESP_DIR;
		CH_PD_DIR;
		CH_PD_SET(1);
		
		RST_ESP_SET(1);
		RST_ESP_SET(0);
		RST_ESP_SET(1);
		esp_power_up = 0;
	}

    while(1)
    {
		while(reset_check_state < reset_check_nr_of_states) {
			switch(reset_check_state) {
				case reset_check_not_done:
					reset_check_state++;
					while(timer_system_ms < RESET_CONFIG_CHECK_START_TIME) {/*Wait*/}
				break;
				
				case reset_checking:
					reset_check_state++;
					if(reset_check()) { //check for reset count
						STATUS_LED_ON;
					}
				break;
				
				case reset_check_done:
					if(timer_system_ms > RESET_CONFIG_CHECK_END_TIME) { //finish reset checking
						reset_clear();
						reset_check_state++;
						animation_play_startup();
						STATUS_LED_OFF;
					}
					
				break;
				
				default:
				break;
			}
			timer_delay_ms(10);
		}
		#if PWM_TERMINAL_CONTROL
			manual_control();
		#else
			#if LIGHTING_WIFI_CONTROL
				#if ESP_TERMINAL_CONTROL
					//do nothing with ESP
				#else
					esp_state_machine();
				#endif //ESP_TERMINAL_CONTROL
			#endif //LIGHTING_WIFI_CONTROL		
		#endif  //PWM_TERMINAL_CONTROL
	}
}
