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

#define ESP_STATE_INIT 0
#define ESP_STATE_SETMODE 1
#define ESP_STATE_CONNECT 2
#define ESP_STATE_CONNECT_RESPONSE 3
#define ESP_STATE_CONNECT_MUX 4
#define ESP_STATE_CONNECT_CLOSE_EXIST_CONNECTION 5
#define ESP_SET_UDP_SERVER 6
#define ESP_STATE_WAITIP 7
#define ESP_STATE_TCPCONNECT 8
#define ESP_STATE_SUCCESS 255

#define true  1
#define false 0
/************************************************************************/
/*                           Global variables                           */
/************************************************************************/
char serialResult[SERIAL_RESULT_BUFFER_SIZE];
volatile uint8_t ESP_CURRENT_STATE = 0;
uint8_t ESP_WIFI_CONNECTED = false;

//volatile uint32_t timer_system_ms = 0;
extern volatile uint32_t timer_counter_target_ms;
/************************************************************************/
/*                      Wifi UART interface functions                   */
/************************************************************************/
#if 1

uint8_t receive_serial()
{
	memset(serialResult, 0, SERIAL_RESULT_BUFFER_SIZE-1);
	#if ESP_DEBUG
	Report_millisec(); uart_send_string(" WAIT INCOMING <--\r\n");
	#endif

	timer_counter_setup(4);
	while(uart_rx_buflen() == 0 && timer_counter_running());
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
				//delayMilliseconds(500);
				timer_delay_ms(500);
				return receive_serial();
			}
			else if(strstr(serialResult, "WIFI DISCONNECT"))
			{
				#if ESP_DEBUG
				Report_millisec(); uart_send_string(" WIFI State change:"); uart_send_string(serialResult); uart_send_string("<--\r\n");
				#endif
				return receive_serial();
			}
			else if(strstr(serialResult, "WIFI GOT IP"))
			{
				#if ESP_DEBUG
				Report_millisec(); uart_send_string(" WIFI State change:"); uart_send_string(serialResult); uart_send_string("<--\r\n");
				#endif
				if(ESP_CURRENT_STATE <= ESP_STATE_CONNECT_RESPONSE)
				{
					ESP_CURRENT_STATE = ESP_STATE_CONNECT_MUX;
					ESP_WIFI_CONNECTED = true;
					return true;
				}
				return receive_serial();
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


static uint8_t checkReturn(char *compareWord)
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


static uint8_t checkUntilTimeout(char *compareWord, uint8_t maxWaitTime)
{
	uint32_t timeoutTimestamp  = timer_ms() + (maxWaitTime * 1000);
	do {
		if(checkReturn(compareWord)) {
			return true;		
		}
	} while(timer_ms() < timeoutTimestamp);
	return false;
}

static uint8_t sendCommand(char *sentCommand, char *compareWord)
{
	uart_send_string(sentCommand); uart_send_char('\r'); uart_send_char('\n');
	if(!checkUntilTimeout(sentCommand, 1))
	{
		return false;		
	}	
	return checkUntilTimeout(compareWord, 1);
}


#endif

void wifi_init(void)
{
	uint8_t connection_established = 0;
	char *temp_resp = NULL;
	
	//Set the direction and value of ESP 8266 Reset (RST) and Enable (CH_PD) pin
	RST_ESP_DIR;
	CH_PD_DIR;
	RST_ESP_SET(1);
	RST_ESP_SET(0);
	RST_ESP_SET(1);
	CH_PD_SET(1);
	
#if 1 //Temporary switch to enable initialization code
	timer_delay_ms(2000);
	
	//do {
		//Check & Set wifi mode
		#if 0
		get_querry("AT+CWMODE_CUR?",temp_resp);
		if(strstr(temp_resp,"+CWMODE_CUR:1") == '\0' ) //Check if Station Mode is set
		{
			send_command("AT+CWMODE=1", temp_resp);  //Set Station mode in flash memory
			//Setting temporary mode might not be necessary
			//send_command("AT+CWMODE_CUR=1", &temp_resp);  //Set Station mode
		}

		//Check & Set access point
		get_querry("AT+CWJAP_CUR?",temp_resp);
		if(strstr(temp_resp,"No AP") == '\0' ) //Check if there is no connection to any AP
		{
			//Might be necessary to have a very long delay between setting AP and OK received
			//If this function is not working, send command separately, wait with timer1, get response
			send_command(strcat("AT+CWJAP_CUR",WIFI_SSID_PASSWORD),temp_resp); //Set AP in flash memory
			//Setting temporary mode might not be necessary
			//send_command("AT+CWJAP_CUR"+WIFI_SSID+WIFI_PASSWORD,&temp_resp); //Set AP
		}
		
		//Check the wifi state
		get_querry("AT+CIPSTATUS",temp_resp);
		#endif
		
		uart_send_string("AT+CWMODE=1\r\n");
		timer_delay_ms(10);
		uart_send_string(strcat("AT+CWJAP=",WIFI_SSID_PASSWORD));
		timer_delay_ms(5000);
		//uart_get_string(temp_resp,100);
		timer_delay_ms(10);
		//uart_send_string(temp_resp);
				
		//Might an alternative to check if state is not 5, meaning NOT connect to an AP
		//if(strstr(temp_resp,"STATUS:5") != '\0' )  // True if string is found in temp_resp
		/*
		if(strstr(temp_resp,"STATUS:2") == '\0' ) //Check if ESP connected to an AP and its IP is obtained
		{
			connection_established = 0;  //Not connected
		}
		else
		{
			connection_established = 1;
		}	*/	
	//} while(!connection_established);
#endif
}

/*
Wifi init AT commands to implement:

	Command							|	Description							|	Saved in
	***************************************************************************************
	AT+CWMODE=1						|	Set WiFistation mode				|	Flash
	AT+CWMODE_CUR=1					|	Set WiFistation mode				|	Temporary
	AT+CWJAP_DEF="SSID","Password"	|	Connects to an AP					|	Flash
	AT+CWJAP_CUR="SSID","Password"	|	Connects to an AP					|	Temporary
	AT+CIPSTATUS					|	Gets the ESP status on the network	|
	AT+CIFSR						|	Get IP Address						|
*/