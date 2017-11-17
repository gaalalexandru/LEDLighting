/*
 * wifi_handler.c
 *
 * Created: 10/17/2017 9:57:41 PM
 *  Author: Gaal Alexandru
 */ 
//Wifi module type: ESP8266

#include <avr/io.h>
#include <avr/portpins.h>
#include <string.h>
#include "configuration.h"
#include "uart_handler.h"
#include "timer_handler.h"
#include "esp_wifi_handler.h"

// Pin mapping for ESP8266 wifi module reset (RST_ESP) and enable (CH_PD), 
// pins have to be digital output
// CH_PD: Chip enable. Keep it on high (3.3V) for normal operation
// RST_ESP: Reset. Keep it on high (3.3V) for normal operation. Put it on 0V to reset the chip.
// RST_ESP mapped to MOSI programing pin PB3
// CH_PD mapped MISO programing pin PB4
#define RST_ESP_DIR	DDRB |= (1 << PIN3)
#define	CH_PD_DIR	DDRB |= (1 << PIN4)
#define RST_ESP_SET(x)	PORTB |= ((x) << PIN3)
#define	CH_PD_SET(x)	PORTB |= ((x) << PIN4)  

#define ESP_DEBUG (0)
#define SERIAL_RESULT_BUFFER_SIZE 101

//Preliminary ESP states
#define ESP_STATE_INIT			0
#define ESP_STATE_SETMODE		1
#define ESP_STATE_CONNECT		2
#define ESP_STATE_CHECK_IP		3
#define ESP_STATE_CHECK_STATUS	4
#define ESP_STATE_SETMUX		5
#define ESP_STATE_START_TCP_SERVER	6
#define ESP_STATE_WAIT_COMMANDS		7

#define true  1
#define false 0

/* To define the maximum waiting time for a response*/
#define SET_RESPONSE_TIMEOUT(x)	(response_max_timestamp =  (timer_ms() + ((x) * 1000)))

/* Will return true if timeout expired*/
#define WAITING_RESPONSE()	(response_max_timestamp > timer_ms())

/************************************************************************/
/*                           Global variables                           */
/************************************************************************/
char serialResult[SERIAL_RESULT_BUFFER_SIZE];
volatile uint8_t ESP_CURRENT_STATE = 0;
volatile uint32_t response_max_timestamp;

/************************************************************************/
/*                      Wifi UART interface functions                   */
/************************************************************************/
uint8_t receive_serial()
{
	memset(serialResult, 0, SERIAL_RESULT_BUFFER_SIZE-1);
	#if ESP_DEBUG
	Report_millisec(); uart_send_string(" WAIT INCOMING <--\r\n");
	#endif

	SET_RESPONSE_TIMEOUT(4);
	while(uart_rx_buflen() == 0 && WAITING_RESPONSE());
	if(uart_rx_buflen() > 0)
	{
		uart_get_string(serialResult, SERIAL_RESULT_BUFFER_SIZE-1);
		if(strlen(serialResult))
		{ //sometime it return empty string :/
			#if ESP_DEBUG
			Report_millisec(); uart_send_string("RECEIVE:"); uart_send_string(serialResult);uart_send_string("<--\r\n");
			#endif

			if(strstr(serialResult, "busy p..") != NULL)
			{
				#if ESP_DEBUG
				Report_millisec(); uart_send_string(" BUSY <--\r\n");
				#endif
				timer_delay_ms(500);
				return receive_serial();  //Try again
			}
			return true;
		} 
		else
		{
			#if ESP_DEBUG
			Report_millisec(); uart_send_string(" EMPTY STRING <--\r\n");
			#endif
			timer_delay_ms(100);
			return receive_serial(); // so, we've to try again
		}
	}
	#if ESP_DEBUG
	else if(!interruptDelayUnFinish())
	{
		Report_millisec(); uart_send_string(" Timeout <--\r\n");
	}
	#endif
	return false;
}


static uint8_t check_return(char *compareWord)
{
	if(receive_serial())
	{
		#if ESP_DEBUG
		Report_millisec(); uart_send_string("Compare:"); uart_send_string(serialResult); uart_send_string(" WITH:"); uart_send_string(compareWord); uart_send_string("<--\r\n");
		#endif

		if(strstr(serialResult, compareWord) != NULL)
		{
			return true;
		}
		else	
		{
			return false;
		}
	}
	else
	{
		#if ESP_DEBUG
		Report_millisec(); uart_send_string(" Return:"); uart_send_string(serialResult); uart_send_string("<--\r\n");
		#endif
		return false;
	}
}


static uint8_t check_until_timeout(char *compareWord, uint8_t maxWaitTime)
{
	uint32_t timeoutTimestamp  = timer_ms() + (maxWaitTime * 1000);
	do {
		if(check_return(compareWord)) {
			return true;		
		}
	} while(timer_ms() < timeoutTimestamp);
	return false;
}

static uint8_t send_command(char *sentCommand, char *compareWord)
{
	uart_send_string(sentCommand); uart_send_char('\r'); uart_send_char('\n');
	if(!check_until_timeout(sentCommand, 1))
	{
		return false;		
	}	
	return check_until_timeout(compareWord, 1);
}

void esp_init(void)
{
	//Set the direction and value of ESP 8266 Reset (RST) and Enable (CH_PD) pin
	RST_ESP_DIR;
	CH_PD_DIR;
	RST_ESP_SET(1);
	RST_ESP_SET(0);
	RST_ESP_SET(1);
	CH_PD_SET(1);
	timer_delay_ms(4000);  //Wait 4 second until ESP is started and finishes standard junk output :)
}

void esp_state_machine(void)
{
	static uint8_t retry_connect = 0;
	while( ESP_CURRENT_STATE < ESP_STATE_WAIT_COMMANDS) 
	{
		switch (ESP_CURRENT_STATE)
		{
			case ESP_STATE_INIT:
				//Synchronize ATMEGA8 with ESP8266
				if(send_command("AT", "OK"))
				{
					ESP_CURRENT_STATE = ESP_STATE_SETMODE;
					uart_flush();
				}	
			break;
			case ESP_STATE_SETMODE:
				//Set ESP8266 mode (1 = Station, 2 = Soft Access Point, 3 = Sta + SoftAP)
				
				//AleGaa: Since CWMODE is set in flash memory of ESP, might be useful 
				//to set the mode only at "manufacturing" via serial terminal
				//and do not set it on runtime.
				//this could make the State Machine slightly simpler
				//feature to be implemented TODO
				
				if(send_command("AT+CWMODE=3", "OK"))
				{
					ESP_CURRENT_STATE = ESP_STATE_CONNECT;
					uart_flush();
				}
			break;
			case ESP_STATE_CONNECT:
				//Set Access Point SSID and Password
				
				//AleGaa: Currently SSID and password is set manually in code
				//for future versions this will be inputed via serial interface
				//or wifi through mobile device connecting to SoftAP of ESP
				//feature to be implemented TODO
				//uint8_t *wifi_credentials = strcat(WIFI_SSID,WIFI_PASSWORD);			
				//if(send_command(strcat("AT+CWMODE=3",wifi_credentials), "OK"))
				
				//send_command not used, because it's necessary to wait a little
				//before checking for OK response
				uart_flush();
				uart_send_string(strcat("AT+CWJAP=",WIFI_SSID_PASSWORD));
				timer_delay_ms(1000);
				if(check_until_timeout("OK",5))
				//if(send_command(strcat("AT+CWJAP=",WIFI_SSID_PASSWORD),"OK"))
				{
					//timer_delay_ms(4000);
					ESP_CURRENT_STATE = ESP_STATE_CHECK_IP;
				}
				else 
				{
					/*uart_send_string("AT+CWQAP\n\r");*/
					timer_delay_ms(100);
					ESP_CURRENT_STATE = ESP_STATE_INIT;
				}
				uart_flush();
			break;			
			case ESP_STATE_CHECK_IP:		
				//Check if ESP received IP from AP
				//if(send_command("AT+CIFSR","+CIFSR:STAIP,\"0.0.0.0\""))
				if(send_command("AT+CIFSR","OK"))  
				//To be replaced with an invalid IP like 0.0.0.0 
				//or a piece of the expected IP, like 192.168.
				//and re-think logic of IF - True - Else statement
				{
					//IP received, meaning successful connection to AP
					//uart_send_string("Connected to AP");
					ESP_CURRENT_STATE = ESP_STATE_CHECK_STATUS;
				}
				else
				{
					//This will be true if no IP received from AP
					//uart_send_string("Did not connect to AP");
					ESP_CURRENT_STATE = ESP_STATE_CONNECT;  //Reset state to connect
					++retry_connect;
					if(retry_connect > 5)
					{
						//AleGaa: In case of unsuccessful connection to AP
						//reset the system or raise a visual warning
						//feature to be implemented TODO
					}					
				}
				uart_flush();
			break;		
			case ESP_STATE_CHECK_STATUS:
				//Check ESP Status (status = 2, means IP received)
				if(send_command("AT+CIPSTATUS", "OK"))
				{
					ESP_CURRENT_STATE = ESP_STATE_SETMUX;
				}
				uart_flush();
			break;
			case ESP_STATE_SETMUX:
				//Set ESP to accept multiple connections
				if(send_command("AT+CIPMUX=1", "OK"))
				{
					ESP_CURRENT_STATE = ESP_STATE_START_TCP_SERVER;
				}
				uart_flush();
			break;
			case ESP_STATE_START_TCP_SERVER:
				//Start TCP server on a manually selected port
				//AleGaa TCP port currently is set manually in code
				//for future versions this will be inputed via serial interface
				//or wifi through mobile device connecting to SoftAP of ESP
				//feature to be implemented TODO
				if(send_command("AT+CIPSERVER=1,1001", "OK"))
				{
					ESP_CURRENT_STATE = ESP_STATE_WAIT_COMMANDS;
				}
				uart_flush();
				//AleGaa maybe move timer0 and pwm initialization here, so that the CPU load during ESP setup is lower
			break;
		}  //end of switch (ESP_CURRENT_STATE)
	}  //end of while( ESP_CURRENT_STATE < ESP_STATE_WAIT_COMMANDS) 


#if 0 //Temporary switch to enable initialization code

		if(send_command("AT", "OK")) {
			STATUS_LED_ON;
		}
		uart_flush();
		
		if(send_command("AT+CWMODE=1", "OK")) {
			STATUS_LED_OFF;
		}
		uart_flush();
		uart_send_string(strcat("AT+CWJAP=",WIFI_SSID_PASSWORD));	
			
		if(check_until_timeout("OK",5))
		{
		} else {
			RST_ESP_SET(1);
			RST_ESP_SET(0);
			RST_ESP_SET(1);
			uart_send_string("AT+RST\r\n");
			timer_delay_ms(15000);
		}
		/*if(send_command(strcat("AT+CWJAP=",WIFI_SSID_PASSWORD),"OK")) {
			STATUS_LED_ON;
		}*/
		uart_flush();		
		if(send_command("AT+CIFSR","OK")) {
			STATUS_LED_ON;
		}
		uart_flush();		
		if(send_command("AT+CWJAP?","OK")) {
			STATUS_LED_ON;
		}
		uart_flush();
		if(send_command("AT+CIPSTATUS","OK")) {
			STATUS_LED_OFF;
		}
		uart_flush();	
		
#endif

}