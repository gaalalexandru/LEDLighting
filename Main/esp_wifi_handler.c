/*
 * wifi_handler.c
 *
 * Created: 10/17/2017 9:57:41 PM
 *  Author: Gaal Alexandru
 */ 
//Wifi module type: ESP8266

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_wifi_handler.h"
#include "configuration.h"
#include "uart_handler.h"
#include "timer_handler.h"
#include "status_led.h"

// Pins have to be digital output
// CH_PD: Chip enable. Keep it on high (3.3V) for normal operation
// RST_ESP: Reset. Keep it on high (3.3V) for normal operation. Put it on 0V to reset the chip.
#define RST_ESP_DIR	ESP_RST_DDR |= (1 << ESP_RST_PIN)
#define	CH_PD_DIR	ESP_ENABLE_DDR |= (1 << ESP_ENABLE_PIN)
#define RST_ESP_SET(x)	((x) ? (ESP_RST_PORT |= (1 << ESP_RST_PIN)) : (ESP_RST_PORT &= ~(1 << ESP_RST_PIN)))
#define	CH_PD_SET(x)	((x) ? (ESP_ENABLE_PORT |= (1 << ESP_ENABLE_PIN)) : (ESP_ENABLE_PORT &= ~(1 << ESP_ENABLE_PIN)))

/*#define ESP_DEBUG (0)*/
#define SERIAL_RESULT_BUFFER_SIZE 101

//Preliminary ESP access point states
#define ESP_AP_INIT				0
#define ESP_AP_SETMUX			1
#define ESP_AP_START_TCP_SERVER	2
#define ESP_AP_CONFIG_RECEIVE	3
#define ESP_AP_CONFIG_CHECK		4
#define ESP_AP_CONFIG_SUCCESS	5

//Preliminary ESP station states
#define ESP_STA_INIT				0
#define ESP_STA_SETMODE				1
#define ESP_STA_CONNECT				2
#define ESP_STA_CHECK_IP			3
#define ESP_STA_CHECK_STATUS		4
#define ESP_STA_SETMUX				5
#define ESP_STA_START_TCP_SERVER	6
#define ESP_STA_WAIT_COMMANDS		7

#define ESP_CHECK_STATUS (0)
#define ESP_FORCE_WIFI_SETUP (1)

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
char workString[32];
char clientIPString[15];
char stationIPString[15];

// most often ID is 0 but we can have up to 4 active connections
// so we want to reply to the right sender who sent ssid and pass
uint8_t senderID = 0;
	
volatile uint8_t esp_sta_current_state = 0;
volatile uint8_t esp_ap_current_state = 0;

char esp_wifi_ssid[ESP_SSID_MAX_LENGTH];
char esp_wifi_pass[ESP_PASS_MAX_LENGTH];

volatile uint32_t response_max_timestamp;
extern volatile uint8_t pwm_width_buffer[CHMAX];
extern volatile status_led_mode_t status_led_mode;

/************************************************************************/
/*                      Wifi UART interface functions                   */
/************************************************************************/
uint8_t receive_serial()
{
	memset(serialResult, 0, SERIAL_RESULT_BUFFER_SIZE-1);
	SET_RESPONSE_TIMEOUT(4);
	
	while(uart_rx_buflen() == 0 && WAITING_RESPONSE()) { /*wait*/ }
	if(uart_rx_buflen() > 0)
	{
		uart_get_string(serialResult, SERIAL_RESULT_BUFFER_SIZE-1);
		if(strlen(serialResult))
		{ //sometime it return empty string :/

			if(strstr(serialResult, "busy p..") != NULL)
			{
				timer_delay_ms(500);
				return receive_serial();  //Try again
			}
			return true;
		} 
		else
		{
			timer_delay_ms(100);
			return receive_serial(); // so, we've to try again
		}
	}
	return false;
}


static uint8_t check_return(char *compareWord)
{
	if(receive_serial())
	{
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

// Creates TCP server and sends data
// destination is the IP to which data is sent
// ID is the connection number
// message is what is sent
inline static void esp_response(uint8_t ID, char *destination, char *message)
{
	uart_send_string("AT+CIPSTART=");
	uart_send_udec(ID);
	uart_send_string(",\"TCP\"");
	uart_send_char(0x2c);	// ,
	uart_send_char(0x22);	// "
	uart_send_string(destination);
	uart_send_char(0x22);	// "
	uart_send_char(0x2c);	// ,
	uart_send_string(ESP_CFG_DEV_PORT);
	uart_newline();
	
	timer_delay_ms(1000);
	
	uart_send_string("AT+CIPSEND=");
	uart_send_udec(ID);
	uart_send_char(0x2c);	// ,
	uart_send_udec(strlen(message));
	uart_newline();
	timer_delay_ms(200);
	uart_send_string(message);
	uart_newline();
}

/************************************************************************/
/*                        Wifi handling functions                       */
/************************************************************************/


void esp_init(void)
{
	//Set the direction and value of ESP 8266 Reset (RST) and Enable (CH_PD) pin
	RST_ESP_DIR;
	CH_PD_DIR;
	RST_ESP_SET(1);
	RST_ESP_SET(0);
	RST_ESP_SET(1);
	CH_PD_SET(1);
	timer_delay_ms(6000);  //Wait 6 second until ESP is started and finishes standard junk output :)
	//Set ESP8266 mode (1 = Station, 2 = Soft Access Point, 3 = Sta + SoftAP)
	//AleGaa: Since CWMODE is set in flash memory of ESP, might be useful
	//to set the mode only at "manufacturing" via serial terminal
	//and do not set it on runtime.
	//this could make the initialization slightly simpler and faster
	//feature to be implemented TODO
	while(!send_command("AT+CWMODE=3", "OK")) {};  //setup AccessPoint and STAtion mode
}
void esp_check_current_setup(void)
{
	char ipCheckResult[SERIAL_RESULT_BUFFER_SIZE];
	uint8_t i = 0;  // index for buffer, and character counter.
	memset(ipCheckResult, 0, SERIAL_RESULT_BUFFER_SIZE-1);
	
	uart_flush();
	uart_send_string("AT+CIFSR\r\n");
	timer_delay_ms(100);
	do
	{
		ipCheckResult[i] = uart_get_char();
		if((ipCheckResult[i] != '\n')  && (ipCheckResult[i] != '\r'))
		{
			i++;			
		}
	}
	while (strstr(ipCheckResult,"STAMAC") == NULL);
	uart_flush();
	
#if ESP_FORCE_WIFI_SETUP  //for development of wifi setup functionalities
	esp_wifi_setup();
#else  //normal run
	/*
	To check if we are already connected to a wifi network:
	Check if string contains station IP nr. 0.0.0.0.
	If string doesn't contain "STAIP,\"0.0.", pointer of strstr().
	will be NULL and the condition will be true.
	In this case we can proceed to MUX setting and TCP server start.
	If string contains "STAIP,\"0.0.", pointer off strstr() not NULL,
	the condition will be false and will start the routines for new
	wifi network setup.
	*/
	if(strstr(ipCheckResult, "STAIP,\"0.0.") == NULL)  //esp station has IP
	{
		//Start ESP State Machine
		//Go directly to MUX setting
		esp_sta_current_state = ESP_STA_SETMUX;
	}
	else  //esp station has no IP
	{
		//Start ESP Access Point and get wifi configuration from mobile device
		esp_wifi_setup();
	}
#endif //ESP_FORCE_WIFI_SETUP	
}

// this function will setup the ESP Access Point
// mobile device will connect to this network
// mobile device will send the SSID and password of home network
// ESP will send to mobile device the IP of ESP on home network
void esp_wifi_setup(void)
{
	char ipCheckResult[SERIAL_RESULT_BUFFER_SIZE];
	uint8_t i = 0;  // index for buffer, and character counter.
	memset(ipCheckResult, 0, SERIAL_RESULT_BUFFER_SIZE-1);
	char *currStrPos = NULL;
	
	uint8_t OK = 0;
	uint8_t FAIL = 0;

	// used in multiple places so we don't use too much memory
	memset(workString, 0, 32);
	
	// changed as global varaiables
	// local variables acted strangely
	memset(clientIPString, 0, 15);
	memset(stationIPString, 0, 15);
	
	char *stationIP_begin = NULL;
	char *stationIP_end = NULL;
	char *clientIP_begin = NULL;
	char *clientIP_end = NULL;
		
	while( esp_ap_current_state < ESP_AP_CONFIG_SUCCESS)
	{
		switch (esp_ap_current_state)
		{
			case ESP_AP_INIT:
				//Synchronize ATMEGA8 with ESP8266
				if(send_command("AT", "OK"))
				{
					//esp_sta_current_state = ESP_AP_SETMODE;
					//AleGaa: skip setmode command for now.
					esp_ap_current_state = ESP_AP_SETMUX;
				}
				uart_flush();
			break;
			case ESP_AP_SETMUX:
				//Set ESP to accept multiple connections
				if(send_command("AT+CIPMUX=1", "OK"))
				{
					//why was this here? todo: uart_send_string(serialResult);
					esp_ap_current_state = ESP_AP_START_TCP_SERVER;
				}
				uart_flush();
			break;			
			case ESP_AP_START_TCP_SERVER:
				//Start TCP server on a manually selected port
				//if(send_command("AT+CIPSERVER=1,1002", "OK"))
				if(send_command(strcat("AT+CIPSERVER=1,",ESP_AP_PORT), "OK"))
				{
					send_command("AT+CIPDINFO=1", "OK");	// detailed information (IP & PORT) in +IPD
					send_command("AT+CIPSTO=60", "OK");		// time until tcp server connection is closed
					esp_ap_current_state = ESP_AP_CONFIG_RECEIVE;
				}
				uart_flush();
			break;
			case ESP_AP_CONFIG_RECEIVE:
				/*
				1. mobile device -> ESP: #\"SSID\",\"PASSWORD"\
				2a. CURRENT VERSION: ATMEGA store to 1 big string the SSID, PASSWORD in the req format
				2b. FEATURE VERSION (TO DO): ATMEGA store to separate string the SSID, PASSWORD, PORT
				3. ESP connect to SSID with PASSWORD (enable station mode)
				4. ESP check IP, if OK go to 5, if not OK, go to 1.
				5. ESP -> mobile device: #stationIP
				*/
				
				OK = 0;
				FAIL = 0;
				
				if(check_until_timeout("+IPD,", 5))
				{
					//response example:
					//+IPD,0,25,192.168.4.2,50511:#"FELINVEST","1234qwe$"
					currStrPos = strstr(serialResult, "+IPD,");  //find start of response
					currStrPos += 5;	// jump to ID of sender
					senderID = atoi(currStrPos);	// store ID of sender
					currStrPos += 2;  //jump to "," before message length
					
					clientIP_begin = strchr(currStrPos, 0x2c);	//jump to next ','
					clientIP_begin++;  //jump to start of IP
					clientIP_end = strchr(clientIP_begin, 0x2c);	//jump to next ','
					strncpy(clientIPString, clientIP_begin, (uint8_t)(clientIP_end-clientIP_begin));
					
					currStrPos = strchr(currStrPos, '#');  //find start of SSID
					currStrPos++;
					strcpy(workString, currStrPos);		// at this point, workString contains SSID and PASS
		
					esp_ap_current_state = ESP_AP_CONFIG_CHECK;
				}
				uart_flush();
			break;
			
			case ESP_AP_CONFIG_CHECK:
				//connect to new SSID
				uart_flush();
				timer_delay_ms(1000);
				uart_send_string("AT+CWJAP=");	// at this point, workString contains SSID and PASS
				uart_send_string(workString);
				uart_newline();
				timer_delay_ms(1000);
				
				do
				{
					uart_get_string(workString, 32);	// at this point, workString contains response from AT+CWJAP
					if(strstr(workString, "OK") != NULL)
					{
						OK = 1;
					}
					else if(strstr(workString, "FAIL") != NULL)
					{
						FAIL = 1;
					}				
				} while ((OK == 0) && (FAIL == 0));
				
				// if esp succeeded in connecting to ssid and pass provided
				if(OK == 1)
				{
					timer_delay_ms(1000);
					uart_flush();
					uart_send_string("AT+CIFSR\r\n");
					timer_delay_ms(100);
					do
					{
						ipCheckResult[i] = uart_get_char();
						if((ipCheckResult[i] != '\n')  && (ipCheckResult[i] != '\r'))
						{
							i++;
						}
					}
					while (strstr(ipCheckResult,"STAMAC") == NULL);
					if(strstr(ipCheckResult, "STAIP,\"0.0.") == NULL)  //esp station has IP
					{
						stationIP_begin = strstr(ipCheckResult, "STAIP");
						stationIP_begin += 7;
						stationIP_end = strchr(stationIP_begin, 0x22);	// ' " '
						uart_flush();	
						strncpy(stationIPString, stationIP_begin, (uint8_t)(stationIP_end-stationIP_begin));
						esp_response(senderID, clientIPString, stationIPString);
						timer_delay_ms(3000);
						
						esp_ap_current_state = ESP_AP_CONFIG_SUCCESS;
					}
					// not needed here since this is treated below in (FAIL == 1) condition and there two possible replies, OK and FAIL
					#if 0
					else
					{
						esp_response(senderID, clientIPString, "Could not connect");
						esp_ap_current_state = ESP_AP_CONFIG_RECEIVE;
					}
					#endif
				}
				
				// if esp failed in connecting to ssid and pass provided, go back to receive ssid and pass state
				else if(FAIL == 1)
				{
					esp_response(senderID, clientIPString, "Could not connect");
					timer_delay_ms(1000);
					esp_ap_current_state = ESP_AP_CONFIG_RECEIVE;
				}
				
			uart_flush();
			break;
		}
	}
	
	if (esp_ap_current_state == ESP_AP_CONFIG_SUCCESS)
	{
		//maybe start state machine???
		esp_sta_current_state = ESP_STA_START_TCP_SERVER;
	}
	
	stationIP_begin = NULL;
	stationIP_end = NULL;
	clientIP_begin = NULL;
	clientIP_end = NULL;
}
/*

*/
void esp_state_machine(void)
{
	static uint8_t retry_connect = 0;
	char *currStrPos, *dataPtr;
	uint8_t channel_nr = 0;
	uint8_t channel_value = 0;
	
	while( esp_sta_current_state < ESP_STA_WAIT_COMMANDS) 
	{
		switch (esp_sta_current_state)
		{
			case ESP_STA_INIT:
				//Synchronize ATMEGA8 with ESP8266
				if(send_command("AT", "OK"))
				{
					esp_sta_current_state = ESP_STA_CONNECT/*ESP_STA_SETMODE*/;
					uart_flush();
					status_led_mode = wait_for_ip;
				}	
			break;
			/*case ESP_STA_SETMODE:

				
				if(send_command("AT+CWMODE=3", "OK"))
				{
					esp_sta_current_state = ESP_STA_CONNECT;
					uart_flush();
				}
			break;*/
			case ESP_STA_CONNECT:
				//Set Access Point SSID and Password
				
				//AleGaa: Currently SSID and password is set manually in code
				//for future versions this will be inputed via serial interface
				//or wifi through mobile device connecting to SoftAP of ESP
				//feature to be implemented TODO
				//uint8_t *wifi_credentials = strcat(WIFI_SSID,WIFI_PASSWORD);			
				//if(send_command(strcat("AT+CWMODE=3",wifi_credentials), "OK"))
				
				//send_command not used, because it's necessary to wait a little longer
				//before checking for OK response
				uart_flush();
				uart_send_string(strcat("AT+CWJAP=",WIFI_SSID_PASSWORD));
				timer_delay_ms(1000);
				if(check_until_timeout("OK",5))
				//if(send_command(strcat("AT+CWJAP=",WIFI_SSID_PASSWORD),"OK"))
				{
					//timer_delay_ms(4000);
					esp_sta_current_state = ESP_STA_CHECK_IP;
				}
				
				else 
				{
					/*uart_send_string("AT+CWQAP\n\r");*/
					timer_delay_ms(100);
					esp_sta_current_state = ESP_STA_INIT;
				}
				uart_flush();
			break;			
			case ESP_STA_CHECK_IP:		
				//Check if ESP received IP from AP
				//if(send_command("AT+CIFSR","+CIFSR:STAIP,\"0.0.0.0\""))
				if(send_command("AT+CIFSR","OK"))  
				//To be replaced with an invalid IP like 0.0.0.0 
				//or a piece of the expected IP, like 192.168.
				//and re-think logic of IF - True - Else statement
				{
					//IP received, meaning successful connection to AP
					#if ESP_CHECK_STATUS
					esp_sta_current_state = ESP_STA_CHECK_STATUS;
					#else
					esp_sta_current_state = ESP_STA_SETMUX;
					#endif  //ESP_CHECK_STATUS
				}
				else
				{
					//This will be true if no IP received from AP
					//Did not connect to AP
					esp_sta_current_state = ESP_STA_CONNECT;  //Reset state to connect
					retry_connect++;
					if(retry_connect > 5)
					{
						status_led_mode = error_indication;
						//AleGaa: In case of unsuccessful connection to AP
						//reset the system or raise a visual warning
						//feature to be implemented TODO
					}					
				}
				uart_flush();
			break;
			#if ESP_CHECK_STATUS
			case ESP_STA_CHECK_STATUS:
				//Check ESP Status (status = 2, means IP received)
				if(send_command("AT+CIPSTATUS", "OK"))
				{
					esp_sta_current_state = ESP_STA_SETMUX;
				}
				uart_flush();
			break;
			#endif  //ESP_CHECK_STATUS
			case ESP_STA_SETMUX:
				//Set ESP to accept multiple connections
				if(send_command("AT+CIPMUX=1", "OK"))
				{
					uart_send_string(serialResult);
					esp_sta_current_state = ESP_STA_START_TCP_SERVER;
				}
				uart_flush();
			break;
			case ESP_STA_START_TCP_SERVER:
				//Start TCP server on a manually selected port
				//AleGaa TCP port currently is set manually in code
				//for future versions this will be inputed via serial interface
				//or wifi through mobile device connecting to SoftAP of ESP
				//feature to be implemented TODO
				if(send_command("AT+CIPSERVER=1,1001", "OK"))
				{
					esp_sta_current_state = ESP_STA_WAIT_COMMANDS;
				}
				uart_flush();
				//AleGaa TODO maybe move timer0 and pwm initialization here, so that the CPU load during ESP setup is lower
			break;
		}  //end of switch (esp_sta_current_state)
	}  //end of while( esp_sta_current_state < ESP_STATE_WAIT_COMMANDS)
	
	//Command format sent through TCP:
	//"+IPD,x,y:$H#DD"
	//x = connection number
	//y = number of characters in message, after ":"
	//$ = start of channel number character(s)
	//H = HEX value of PWM channel number to be changed
	//# = start of PWM duty cycle value character(s)
	//DD = HEX value of PWM duty cycle for selected channel
	//		00 = 0%, 7F = 50%, FF = 100%
	if( esp_sta_current_state == ESP_STA_WAIT_COMMANDS) 
	{
		status_led_mode = connected_to_ap;
		if(check_until_timeout("+IPD,", 5))
		{
			currStrPos = strstr(serialResult, "+IPD,");
			currStrPos += 5;
			currStrPos = strchr(currStrPos, '$');  //find start of channel byte
			dataPtr = currStrPos + 1;
			channel_nr = (((uint8_t)*dataPtr) - 0x30);  //save the target channel nr.*/
			//channel_nr = (*dataPtr);  //save the target channel nr.*/
						
			currStrPos = strchr(currStrPos, '#');  //find start of duty cycle byte		
			dataPtr = currStrPos + 1;

			channel_value = *dataPtr;  //save the target duty cycle value
			pwm_width_buffer[channel_nr] = channel_value; // update pwm_width buffer
			
			uart_send_string("pwm ");
			uart_send_udec(channel_value);
			uart_newline();		
		}
		
		uart_flush();
	}
}