/*
 * wifi_handler.c
 *
 * Created: 10/17/2017 9:57:41 PM
 *  Author: Gaal Alexandru, Bogdan Rat
 */ 
//Wifi module type: ESP8266

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "esp_wifi_handler.h"
#include "esp_wifi_handler_defs.h"
#include "configuration.h"
#include "uart_handler.h"
#include "timer_handler.h"
#include "status_led.h"
#include "animation_handler.h"
#include "pwm_handler.h"
#include "eeprom_handler.h"
#include "errors.h"
#include "reset_handler.h"

// Pins have to be digital output
// CH_PD: Chip enable. Keep it on high (3.3V) for normal operation
// RST_ESP: Reset. Keep it on high (3.3V) for normal operation. Put it on 0V to reset the chip.
#define RST_ESP_DIR	ESP_RST_DDR |= (1 << ESP_RST_PIN)
#define	CH_PD_DIR	ESP_ENABLE_DDR |= (1 << ESP_ENABLE_PIN)
#define RST_ESP_SET(x)	((x) ? (ESP_RST_PORT |= (1 << ESP_RST_PIN)) : (ESP_RST_PORT &= ~(1 << ESP_RST_PIN)))
#define	CH_PD_SET(x)	((x) ? (ESP_ENABLE_PORT |= (1 << ESP_ENABLE_PIN)) : (ESP_ENABLE_PORT &= ~(1 << ESP_ENABLE_PIN)))

/* To define the maximum waiting time for a response*/
#define SET_RESPONSE_TIMEOUT(x)	(response_max_timestamp =  (timer_ms() + ((x) * 1000)))
/* Will return true if timeout expired*/
#define WAITING_RESPONSE()	(response_max_timestamp > timer_ms())

/************************************************************************/
/*                           Global variables                           */
/************************************************************************/
volatile uint8_t esp_is_connected = false;	// ESP has ip or not
char esp_wifi_credentials[BUFFER_SIZE_WIFI_CREDENTIALS_STRING];
char esp_serial_result[BUFFER_SIZE_SERIAL_RESULT];
char esp_station_IP[BUFFER_SIZE_IP_STRING];
// most often ID is 0 but we can have up to 4 active connections
// so we want to reply to the right sender who sent ssid and pass
//volatile uint8_t esp_connection_ID = 0;  //not used anymore, replaced by local in esp_state_machine, leave for feature reference
//char esp_client_IP[BUFFER_SIZE_IP_STRING];  //not used anymore, replaced by local in esp_state_machine, leave for feature reference
volatile uint32_t response_max_timestamp;
extern volatile uint8_t pwm_width_buffer[PWM_CONFIG_CHMAX];
extern volatile status_led_mode_t status_led_mode;
volatile bool esp_power_up = 1;
/************************************************************************/
/*                      Wifi UART interface functions                   */
/************************************************************************/

uint8_t receive_serial()
{
	memset(esp_serial_result, 0, BUFFER_SIZE_SERIAL_RESULT-1);
	SET_RESPONSE_TIMEOUT(4);
	
	while(uart_rx_buflen() == 0 && WAITING_RESPONSE()) { /*wait*/ }
	if(uart_rx_buflen() > 0)
	{
		uart_get_string(esp_serial_result, BUFFER_SIZE_SERIAL_RESULT-1);
		if(strlen(esp_serial_result))
		{ //sometime it return empty string :/

			if(strstr(esp_serial_result, "busy p..") != NULL)
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
		if(strstr(esp_serial_result, compareWord) != NULL)
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
/************************************************************************/
/*                     ESP / WiFi handling functions                    */
/************************************************************************/

//helper function that extracts station IP
//extracted data is stored in global variables
inline static void esp_aux_calc_station_ip(char *workString)
{
	char *stationIP_begin = NULL;
	char *stationIP_end = NULL;
	stationIP_begin = strstr(workString, "STAIP");
	stationIP_begin += 7;
	stationIP_end = strchr(stationIP_begin, 0x22);	// ' " '
	uart_flush();
	/*
	 * It's possible that in the ESP Station changes it's IP due to new network for example
	 * The string containing the IP should be re-initialized to empty string before storing new IP
	 * because old IP might have more characters, and characters over the new IP length will remain.
	 * Example: OLD IP: 192.168.43.145; New IP: 192.168.1.110; Reported(Stored) IP: 192.168.1.1105
	 */
	memset(esp_station_IP,0,BUFFER_SIZE_IP_STRING);
	strncpy(esp_station_IP, stationIP_begin, (uint8_t)(stationIP_end-stationIP_begin));
}

//helper function that extracts connection ID and client IP
inline static void esp_aux_client_data(char *workString, uint8_t *connection_ID, char *client_IP)
{
	char *clientIP_begin = NULL;
	char *clientIP_end = NULL;
	
	*connection_ID = atoi(workString);	// store ID of sender
	workString += 2;  //jump to "," before message length
	clientIP_begin = strchr(workString, 0x2c);	//jump to next ','
	clientIP_begin++;  //jump to start of IP
	clientIP_end = strchr(clientIP_begin, 0x2c);	//jump to next ','
	strncpy(client_IP, clientIP_begin, (uint8_t)(clientIP_end-clientIP_begin));
	
	#if 0  //activate to check connection ID and client IP in terminal
	uart_send_udec(*connection_ID);
	uart_send_char('>');
	uart_send_string(client_IP);
	uart_newline();
	#endif
}

// ID is the connection number
// destination is the IP to which data is sent
static void esp_response(uint8_t ID, char *destination, char *message)
{
	//AleGaa The next block is deactivated because we will use persistent TCP connection
	//Note: Rarely strange behavior was observed, sometimes when joining the router wifi network, the connection is closed too early
	#if 0  
	uart_send_string("AT+CIPSTART="); uart_send_udec(ID); uart_send_string(",\"TCP\""); uart_send_char(0x2c); uart_send_char(0x22); 
	uart_send_string(destination); uart_send_char(0x22); uart_send_char(0x2c); uart_send_string(ESP_CONFIG_TCP_PORT); uart_newline();
	#endif
	
	#if 0 //Wait for TCP server success signal before sending data to client
	char workString[32];
	memset(workString, 0, 32);
	do
	{
		uart_get_string(workString, 32);	// at this point, workString contains response from AT+CIPSTART
	} while ((strstr(workString, "OK") == NULL) && (strstr(workString, "ALREADY") == NULL));
	#else //because waiting for OK produced undesired behavior, just wait 2sec.
	timer_delay_ms(2000);
	#endif
	//Send the command and the message length
	uart_send_string("AT+CIPSEND=");
	uart_send_udec(ID);
	uart_send_char(0x2c);	// ,
	uart_send_udec(strlen(message));
	uart_newline();
	//Send the actual message
	timer_delay_ms(200);
	uart_send_string(message);
	uart_newline();
}

static inline uint8_t esp_ap_control(uint8_t u8ap_new_state)
{
	uint8_t u8response = 0;
	if(u8ap_new_state == ESP_SYM_AP_OFF)
	{
		u8response = send_command("AT+CWMODE=1", "OK");
		timer_delay_ms(50);
	}
	else if((u8ap_new_state == ESP_SYM_AP_ON))
	{
		u8response = send_command("AT+CWMODE=3", "OK");
		timer_delay_ms(50);
	}
	else
	{
		uart_send_string(ERROR_ESP_AP_CONTROL_UndefinedAPModeRequested);
	}
	if(u8response)
	{
		u8response = ((u8ap_new_state == ESP_SYM_AP_ON)? ESP_RETURN_AP_ON: \
					 (u8ap_new_state == ESP_SYM_AP_OFF)? ESP_RETURN_AP_OFF: ESP_RETURN_ERROR);
	}
	else
	{
		u8response = ESP_RETURN_ERROR;
		uart_send_string(ERROR_ESP_AP_CONTROL_FailedAPSetting);
	}
	return u8response;
}

uint8_t esp_init_hw(uint16_t u16init_delay)
{	
	uint8_t u8response = ESP_RETURN_NDEF;
	uint8_t u8work_int = 0;
	
	/* HW init procedure:
	 * 1) Only on first startup: set the direction of ESP 8266 Reset (RST) and Enable (CH_PD) pin
	 * 2) Only on first startup: set value of Enable (CH_PD) pin
	 * 3) On every restart set - clear - set Reset (RST) pin
	 * Step 1) and 2) from Version3 will be done in main, to be able to set / clear GPIO pins before using ESP
	 */
	
	//esp first power up will be done at main, leave this here only for feature reference
	#if 0
	if(esp_power_up/*u8first_start*/)
	{
		RST_ESP_DIR;
		CH_PD_DIR;
		CH_PD_SET(1);
		esp_power_up = 0;
	}
	#endif

	
	RST_ESP_SET(1);
	RST_ESP_SET(0);
	RST_ESP_SET(1);
	
	//timer_delay_ms(100);
	//startup animation has to be called after the ESP reset and enable pins are set
	//otherwise ESP might not start up
	//animation_play_startup();
	
	timer_delay_ms(u16init_delay);  //Wait u8init_delay millisecond until ESP is started and finishes standard junk output :)
	//syncronize ESP8266 with ATMEGA8 and set ESP to accept multiple connections
	if(send_command("AT", "OK"))
	{
		u8response = ESP_RETURN_OK;
		} else {
		u8response = ESP_RETURN_ERROR;
		uart_send_string(ERROR_ESP_STATE_HW_INIT_FailedSync);
	}
	if(send_command("AT+CIPMUX=1", "OK"))
	{
		u8response &= ESP_RETURN_OK;
		} else {
		u8response &= ESP_RETURN_ERROR;
		uart_send_string(ERROR_ESP_STATE_HW_INIT_FailedSetCipmux);
	}
	if(eeprom_read_byte(EEL_ADDR_AP_ALWAYS_ON) == ESP_SYM_AP_ALWAYS_ON)
	{
		u8work_int = esp_ap_control(ESP_SYM_AP_ON);
		if(ESP_RETURN_AP_ON == u8work_int) {
			u8response &= ESP_RETURN_OK;
		} else {
			u8response &= ESP_RETURN_ERROR;
		}
	} 
	else if (eeprom_read_byte(EEL_ADDR_AP_ALWAYS_ON) == ESP_SYM_AP_NOT_ALWAYS_ON)
	{
		u8work_int = esp_ap_control(ESP_SYM_AP_OFF);
		if (ESP_RETURN_AP_OFF == u8work_int) {
			u8response &= ESP_RETURN_OK;
		} else { 
			u8response &= ESP_RETURN_ERROR;
		}
	} else {
		uart_send_string(ERROR_ESP_STATE_HW_INIT_UndefinedAPConfigStored);
	}
	return u8response;
}

// fills parameter with the response for AT+CIFSR command
// and also returns ESP_RETURN_CONNECTED / ESP_RETURN_NOT_CONNECTED if station has/hasn't got an IP
static inline uint8_t esp_check_connection(/*char workString[]*/)
{
	/*
	 * To check if we are already connected to a wifi network:
	 * Send command to ESP "AT+CIFSR"
	 * Check if response string contains station IP nr. 0.0.0.0.
	 * If string doesn't contain "STAIP,\"0.0.", pointer of strstr().
	 * will be NULL and the condition will be true, esp has IP.
	 * If string contains "STAIP,\"0.0.", pointer off strstr() not NULL,
	 * the condition will be false, esp has no IP.
	 */
	char workString[BUFFER_SIZE_SERIAL_RESULT];
	uint8_t u8response = ESP_RETURN_NDEF;
	
	memset(workString, 0, BUFFER_SIZE_SERIAL_RESULT-1);
	uint8_t u8index = 0;  // index for buffer, and character counter.
	uart_flush();
	//since the response from AT+CIFS is on many lines
	//can vary and be very long we will not use the send command function
	uart_send_string("AT+CIFSR\r\n");
	timer_delay_ms(200);
	do
	{
		workString[u8index] = uart_get_char();
		if((workString[u8index] != '\n')  && (workString[u8index] != '\r'))
		{
			u8index++;
		}
	}
	while (strstr(workString,"STAMAC") == NULL);
	uart_flush();
	if(strstr(workString, "STAIP,\"0.0.") == NULL)
	{
		u8response = ESP_RETURN_CONNECTED;
	} else {
		u8response = ESP_RETURN_NOT_CONNECTED;
	}
	
	char *stationIP_begin = NULL;
	char *stationIP_end = NULL;
	stationIP_begin = strstr(workString, "STAIP");
	stationIP_begin += 7;
	stationIP_end = strchr(stationIP_begin, 0x22);	// ' " '
	uart_flush();
	
	/*
	 * It's possible that in the ESP Station changes it's IP due to new network for example
	 * The string containing the IP should be re-initialized to empty string before storing new IP
	 * because old IP might have more characters, and characters over the new IP length will remain.
	 * Example: OLD IP: 192.168.43.145; New IP: 192.168.1.110; Reported(Stored) IP: 192.168.1.1105
	 */
	//memset(esp_station_IP,0,BUFFER_SIZE_IP_STRING);  //AleGaa using memset here, produced constant reset???
	strncpy(esp_station_IP, stationIP_begin, (uint8_t)(stationIP_end-stationIP_begin));
	memset(workString, 0, BUFFER_SIZE_SERIAL_RESULT-1);
	return u8response;
}

void esp_state_machine(void)
{
	static uint8_t u8esp_current_state = ESP_STATE_HW_INIT;
	static uint8_t u8connection_retry_count = 0;
	uint8_t u8work_int = 0;
	uint16_t u16work_int = 0xFFFF;
	char *pc_current_string_pos = NULL;
	//char ac_ip_check_result[BUFFER_SIZE_SERIAL_RESULT];
	char ac_work_string[BUFFER_SIZE_GENERIC_WORK_STRING];
	static uint8_t u8connection_ID = 0xFF;
	static char ac_client_IP[BUFFER_SIZE_IP_STRING];
	memset(ac_client_IP, 0, BUFFER_SIZE_IP_STRING);
	memset(ac_work_string, 0, BUFFER_SIZE_GENERIC_WORK_STRING);
	switch(u8esp_current_state)
	{
		case ESP_STATE_HW_INIT:

			
			status_led_mode = wait_for_ip;
			u8work_int = esp_init_hw(ESP_CONFIG_INIT_DELAY);
			timer_delay_ms(100);
			if(ESP_RETURN_OK == u8work_int)
			{
				u8esp_current_state = ESP_STATE_CHECK_CONNECTION;
			}
			uart_flush();
		break;
		
		case ESP_STATE_CHECK_CONNECTION:
			#if ESP_CONFIG_FORCE_WIFI_SETUP
				u8esp_current_state = ESP_STATE_START_AP;
			#else
			memset(esp_station_IP,0,BUFFER_SIZE_IP_STRING);
			u8work_int = esp_check_connection(/*ac_ip_check_result*/);
			if(ESP_RETURN_CONNECTED == u8work_int) {
				u8esp_current_state = ESP_STATE_START_TCP_SERVER;
				esp_is_connected = true;
				status_led_mode = connected_to_ap;
			} else if(ESP_RETURN_NOT_CONNECTED == u8work_int) {
				u8esp_current_state = ESP_STATE_START_AP;
				esp_is_connected = false;
			} else {
				uart_send_string(ERROR_ESP_STATE_CHECK_CONNECTION_UnknownCheckConnResult);
			}
			#endif //ESP_CONFIG_FORCE_WIFI_SETUP
		break;
		
		case ESP_STATE_START_AP:
			u8work_int = esp_ap_control(ESP_SYM_AP_ON);
			if(ESP_RETURN_AP_ON == u8work_int)
			{
				u8esp_current_state = ESP_STATE_START_TCP_SERVER;
			}
			else
			{
				uart_send_string(ERRPR_ESP_STATE_START_AP_FailedStartAP);
			}
			uart_flush();
		break;
		
		case ESP_STATE_START_TCP_SERVER:
			/*
			 * Close previously created server; replies "no change" if none was created before
			 * Start new TCP server on ESP_CONF_TCP_PORT
			 * Get detailed information (IP & PORT) in +IPD
			 * Time until TCP server connection is closed
			 */
			u8work_int = 1;
			if (send_command("AT+CIPSERVER=0", "OK")) {
				u8work_int &= 1;
			} else {
				uart_send_string(ERROR_ESP_STATE_START_TCP_SERVER_FailedOldServerStop);
				u8work_int &= 0;
			}
			timer_delay_ms(400);
			memset(ac_work_string,0,BUFFER_SIZE_GENERIC_WORK_STRING);
			strcpy(ac_work_string,"AT+CIPSERVER=1,");
			strcat(ac_work_string, ESP_CONFIG_TCP_PORT);			
			//if (send_command(strcat("AT+CIPSERVER=1,", ESP_CONFIG_TCP_PORT), "OK")) {
			//if (send_command("AT+CIPSERVER=1,1001", "OK")) {
			if (send_command(ac_work_string, "OK")) {
				u8work_int &= 1;
			} else {
				uart_send_string(ERROR_ESP_STATE_START_TCP_SERVER_FailedServerStart);
				u8work_int &= 0;
			}
			timer_delay_ms(400);
			memset(ac_work_string,0,BUFFER_SIZE_GENERIC_WORK_STRING);
			if (send_command("AT+CIPDINFO=1", "OK")) {
				u8work_int &= 1;
			} else {
				uart_send_string(ERROR_ESP_STATE_START_TCP_SERVER_FailedAdditionalInfoRequest);
				u8work_int &= 0;
			}
			timer_delay_ms(50);
			if (send_command(strcat("AT+CIPSTO=",ESP_CONFIG_TCP_TIMEOUT), "OK")) {
				u8work_int &= 1;
			} else {
				uart_send_string(ERROR_ESP_STATE_START_TCP_SERVER_FailedSterverTimeout);
				u8work_int &= 0;
			}
			timer_delay_ms(50);
			if (u8work_int) {
				u8esp_current_state = ESP_STATE_WAIT_DATA;
			}
			uart_flush();
			u8work_int = 0;
		break;
		
		case ESP_STATE_WAIT_DATA:
			/* 
			 * Incoming data example:
			 * +IPD,0,25,192.168.4.2,50511:@"MyWifiNetworkSSID","MyPassword"
			 * +IPD,0,7,192.168.1.136,64238:$Channel#PowerValue
			 * +IPD,0,7,192.168.1.136,64238:#CommandOption
			 */
			if(check_until_timeout("+IPD,", 1))
			{
				pc_current_string_pos = strstr(esp_serial_result, "+IPD,");
				pc_current_string_pos = strchr(pc_current_string_pos, ',');  //find end of +IPD, the ',' character right before connection ID
				pc_current_string_pos++;
				esp_aux_client_data(pc_current_string_pos, &u8connection_ID, ac_client_IP);

				
				pc_current_string_pos = strchr(pc_current_string_pos, ':');  //find end of client IP, port nr
				pc_current_string_pos++;
				if(*pc_current_string_pos == ESP_SYM_DATA_IS_PWM_CH)  //if we receive a command for PWM setting - command begins with $
				{
					pc_current_string_pos++;
					u8work_int = (*pc_current_string_pos) - 0x30;
					pc_current_string_pos++;
					//check if next character is start of duty cycle byte (#) or still channel nr.
					if(*pc_current_string_pos != ESP_SYM_DATA_IS_PWM_DC)
					{
						u8work_int = (u8work_int*10)+((*pc_current_string_pos) - 0x30);
					}
					pc_current_string_pos = strchr(pc_current_string_pos, ESP_SYM_DATA_IS_PWM_DC);  //find start of duty cycle byte
					pc_current_string_pos++;  //go to duty cycle byte
					if(pwm_wifi_update(u8work_int,  ((uint8_t)*pc_current_string_pos)))
					{
						//pwm duty cycle update was successful (values OK)
						esp_response(u8connection_ID, ac_client_IP, "OK");
					}
					else
					{
						esp_response(u8connection_ID, ac_client_IP, "ERR");
					}
					u8work_int = 0;
				}
				else if(*pc_current_string_pos == ESP_SYM_DATA_IS_CMD) // we receive a command other than PWM setting - command begins with #
				{
					pc_current_string_pos++;
					switch (*pc_current_string_pos)
					{
						case ESP_CMD_GET_STA_IP:  //E command: get STA IP
						memset(esp_station_IP,0,BUFFER_SIZE_IP_STRING);
						if(esp_check_connection(/*ac_ip_check_result*/))  //esp station has IP
						{
							/*esp_aux_calc_station_ip(ac_ip_check_result);*/
							esp_response(u8connection_ID, ac_client_IP, esp_station_IP);
						} else {
							esp_response(u8connection_ID, ac_client_IP, "NO CONN");
						}
						break;
						
						case ESP_CMD_SET_AUTOCONNECT:  //F command: activate / deactivate ESP auto connect to saved network
						pc_current_string_pos++;
						#if 1
						if((*pc_current_string_pos == ESP_SYM_AUTOCONNECT_ON) || \
						(*pc_current_string_pos == ESP_SYM_AUTOCONNECT_OFF)) {
							memset(ac_work_string,0,BUFFER_SIZE_GENERIC_WORK_STRING);
							strcpy(ac_work_string,"AT+CWAUTOCONN=");
							strcat(ac_work_string,pc_current_string_pos);
							if (send_command(ac_work_string,"OK")) {
								eeprom_write_byte(EEL_ADDR_ESP_AUTOCONNECT,*pc_current_string_pos);
								esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"OK"));
							} else {
								esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"ERR"));
							}
							memset(ac_work_string,0,BUFFER_SIZE_GENERIC_WORK_STRING);
						} else {
							esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"NOTDEF"));
						}
						#else
						if(*pc_current_string_pos == '0')  //deactivate auto connect
						{
							send_command("AT+CWAUTOCONN=0", "OK");
							eeprom_write_byte(EEL_ADDR_ESP_AUTOCONNECT,ESP_SYM_AUTOCONNECT_ON);
							esp_response(u8connection_ID, ac_client_IP, "0");
						}
						else if(*pc_current_string_pos == '1') //activate auto connect
						{
							send_command("AT+CWAUTOCONN=1", "OK");
							eeprom_write_byte(EEL_ADDR_ESP_AUTOCONNECT,ESP_SYM_AUTOCONNECT_OFF);
							esp_response(u8connection_ID, ac_client_IP, "1");
						}
						else{ /*do nothing*/ }
						#endif
						
						break;
						
						case ESP_CMD_SET_DEFAULT_PWM:  //G command: set default PWM (will be stored in EEPROM)
						pc_current_string_pos++;
						if(pwm_save_default_dutycycle((uint8_t)*pc_current_string_pos))
						{
							esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"OK"));
						}
						else
						{
							esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"ERR"));
						}
						break;
						
						case ESP_CMD_SET_STARTUP_ANIM:  //H command: set startup animation
						pc_current_string_pos++;
						if(animation_save_startup_anim((uint8_t)*pc_current_string_pos))
						{
							esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"OK"));
						}
						else
						{
							esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"ERR"));
						}
						break;
						
						case ESP_CMD_SET_NO_NETWORK_ANIM:  //I command: set no network animation
						pc_current_string_pos++;
						if(animation_save_no_netw_anim((uint8_t)*pc_current_string_pos))
						{
							esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"OK"));
						}
						else
						{
							esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"ERR"));
						}
						break;

						case ESP_CMD_SET_NO_NETWORK_POWER:  //J command: set no network notification power
						pc_current_string_pos++;
						if(animation_save_no_netw_power((uint8_t)*pc_current_string_pos))
						{
							esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"OK"));
						}
						else
						{
							esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"ERR"));
						}
						break;

						case ESP_CMD_GET_DEVICE_ID: //K command:get the device ID from eeprom
						u8work_int = eeprom_load_id();
						if(u8work_int != EEPROM_INVALID_ID)
						{
							esp_response(u8connection_ID, ac_client_IP, (char*)&u8work_int /*strcat("LL",&u8dev_id)*/);
						}
						else
						{
							esp_response(u8connection_ID, ac_client_IP, "NoID");
						}
						break;

						case ESP_CMD_SET_DEVICE_ID: //L command:save the device ID to eeprom
						pc_current_string_pos++;
						if(eeprom_save_id((uint8_t)*pc_current_string_pos))
						{
							esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"OK"));
						}
						else
						{
							esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"ERR"));
						}
						break;

						case ESP_CMD_GET_DEVICE_SETTINGS: //M command:sync device settings
							//byte 0 -> 11 of response contain the channel PWM duty cycles
							memset(ac_work_string,0,BUFFER_SIZE_GENERIC_WORK_STRING);
							for (u8work_int=0; u8work_int<PWM_CONFIG_CHMAX; u8work_int++)
							{
								*(ac_work_string+u8work_int) = pwm_width_buffer[u8work_int];
							}
							
							//byte 12 contains the ESP AUTOCONN On / Off setting
							u8work_int=PWM_CONFIG_CHMAX;
							*(ac_work_string+u8work_int) = eeprom_read_byte(EEL_ADDR_ESP_AUTOCONNECT);
							
							//byte 13 contains the ESP AP ALWAYS ON On / Off setting
							u8work_int++;
							*(ac_work_string+u8work_int) = eeprom_read_byte(EEL_ADDR_AP_ALWAYS_ON);
							
							//byte 14 contains the StartUpAnimation setting
							u8work_int++;
							*(ac_work_string+u8work_int) =  eeprom_read_byte(EEL_ADDR_STARTUP_ANIMATION);
							
							//byte 15 contains the No Conn Notif
							u8work_int++;
							*(ac_work_string+u8work_int) = eeprom_read_byte(EEL_ADDR_NO_CONN_NOTIFICATION);
							
							//byte 15 contains the No Conn Power
							u8work_int++;
							*(ac_work_string+u8work_int) = eeprom_read_byte(EEL_ADDR_NO_CONN_POWER);

							//byte 16 contains the Default Power
							u8work_int++;
							*(ac_work_string+u8work_int) = eeprom_read_byte(EEL_ADDR_DEFAULT_POWER);
							
							esp_response(u8connection_ID, ac_client_IP, ac_work_string);
							memset(ac_work_string,0,BUFFER_SIZE_GENERIC_WORK_STRING);
						break;
						
						case ESP_CMD_SET_AP_ALWAYS_ON:  //configure AP to be or not to be always on in EEPROM
							pc_current_string_pos++;
							eeprom_write_byte(EEL_ADDR_AP_ALWAYS_ON, (uint8_t)*pc_current_string_pos);  
							//0x30 not always ON, 0x31 always ON	
							if(((*pc_current_string_pos) == ESP_SYM_AP_ALWAYS_ON) || ((*pc_current_string_pos) == ESP_SYM_AP_NOT_ALWAYS_ON))
							{
								esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"OK"));
							}
							else
							{
								esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"ERR"));
							}
						break;

						case ESP_CMD_SET_AP_ON_OFF:  //switch AP on / off
						pc_current_string_pos++;
						u8work_int = esp_ap_control((uint8_t)*pc_current_string_pos);
						
						if(u8work_int)
						{
							esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"OK"));
						}
						else
						{
							esp_response(u8connection_ID, ac_client_IP, strcat(pc_current_string_pos,"ERR"));
						}
						break;
						
						case ESP_CMD_SET_BYTE_EEPROM:  //Generic command to set a byte in EEPROM: #(Addr=2byte)(Data=1Byte)
							pc_current_string_pos++;
							u8work_int = (*pc_current_string_pos)-0x30;  //MSB of address converted from ascii to int with -0x30
							u16work_int = ((uint16_t)u8work_int) << 8;
							
							pc_current_string_pos++;
							u8work_int = (*pc_current_string_pos)-0x30; //LSB of address
							u16work_int |= (uint16_t)u8work_int;
							
							pc_current_string_pos++;
							u8work_int = (*pc_current_string_pos); //data to be written
							
							if (u16work_int < EEPROM_CONFIG_SIZE) {
								#if 0
								uart_send_uhex(u16work_int); uart_send_char('>'); uart_send_uhex(u8work_int); uart_newline();
								#endif
								eeprom_write_byte(u16work_int,u8work_int);
								esp_response(u8connection_ID, ac_client_IP, "DONE");
							} else {
								esp_response(u8connection_ID, ac_client_IP, "ADDR ERR");
							}
							u16work_int = 0xFFFF;
						break;
						
						case ESP_CMD_GET_BYTE_EEPROM:
							pc_current_string_pos++;
							u8work_int = (*pc_current_string_pos)-0x30;  //MSB of address
							u16work_int = ((uint16_t)u8work_int) << 8;
							
							pc_current_string_pos++;
							u8work_int = (*pc_current_string_pos)-0x30; //LSB of address
							u16work_int |= (uint16_t)u8work_int;							
							
							if (u16work_int < EEPROM_CONFIG_SIZE) {
								#if 0
								uart_send_uhex(u16work_int); uart_newline();
								#endif
								u8work_int = eeprom_read_byte(u16work_int);
								esp_response(u8connection_ID, ac_client_IP, (char *)&u8work_int);
							} else {
								esp_response(u8connection_ID, ac_client_IP, "ADDR ERR");
							}
							u16work_int = 0xFFFF;
						break;						

						case ESP_CMD_RESET_EEPROM:
							eeprom_write_byte(EEL_ADDR_FIRST_START,0);
							eeprom_init();
							esp_response(u8connection_ID, ac_client_IP, "DONE");
						break;
						
						case ESP_CMD_RESET_SYSTEM:
							//Use Watchdog Timer and an infinite loop to reset the processor
							esp_response(u8connection_ID, ac_client_IP, "SYS RST");
							reset_controller();
						break;
						
						
						default:
						//do nothing
						break;
					}
					u8work_int = 0;  //reset
					timer_delay_ms(2000);
				}
				else if(*pc_current_string_pos == ESP_SYM_DATA_IS_SSID) //we receive network credentials - command begins with @
				{
					pc_current_string_pos++;
					strcpy(esp_wifi_credentials, pc_current_string_pos);
					u8esp_current_state = ESP_STATE_JOIN_NEW_NETWORK;
				}
			}
			/*
			 * Handling of continuous connection checking:
			 * Check connection only when WIFI DISCONNECT or WIFI CONNECTED 
			 * is found in esp_serial_result up to this point,
			 * receive_serial function was called above
			 * this seems to work best for a beta version
			 * more tests needed to see if fully stable
			 */
			#if ESP_CONFIG_CHECK_RUNTIME_CONNECTION
			//if((strstr(esp_serial_result, "DISCONNECT") != NULL) || (strstr(esp_serial_result, "CONNECTED") != NULL)) {
			/*
			 * Sometimes when network connection was lost (DISCONNECT detected) 
			 * and right after that the connection reestablished, 
			 * the CONNECTED word was not detected.
			 * For this reason for the moment look for word WIFI, 
			 * that should arise only @ disconnect / connect.
			 */
			if(strstr(esp_serial_result, "WIFI") != NULL){
				u8esp_current_state = ESP_STATE_HW_INIT;
			}
			#endif	//ESP_CONFIG_CHECK_RUNTIME_CONNECTION
			uart_flush();
		break;
		
		case ESP_STATE_JOIN_NEW_NETWORK:
			//connect to new SSID
			uart_flush();
			esp_is_connected = false;
			uart_send_string("AT+CWJAP=");
			uart_send_string(esp_wifi_credentials);
			uart_newline();
			timer_delay_ms(1200);
			u8work_int = ESP_RETURN_NDEF;
			
			memset(ac_work_string,0,BUFFER_SIZE_GENERIC_WORK_STRING);
			do
			{
				uart_get_string(ac_work_string, 32);	// at this point, ac_work_string contains response from AT+CWJAP
				if(strstr(ac_work_string, "OK") != NULL)
				{
					u8work_int = ESP_RETURN_CONNECT_SUCCESS;
				}
				else if(strstr(ac_work_string, "FAIL") != NULL)
				{
					u8work_int = ESP_RETURN_CONNECT_FAILED;
				}
			} while ((u8work_int != ESP_RETURN_CONNECT_SUCCESS) && (u8work_int != ESP_RETURN_CONNECT_FAILED));
			memset(ac_work_string,0,BUFFER_SIZE_GENERIC_WORK_STRING);
			// if esp succeeded in connecting to ssid and pass provided
			if(ESP_RETURN_CONNECT_SUCCESS == u8work_int)
			{
				timer_delay_ms(1000);
				memset(esp_station_IP,0,BUFFER_SIZE_IP_STRING);
				u8work_int = esp_check_connection(/*ac_ip_check_result*/);
				if(ESP_RETURN_CONNECTED == u8work_int)  //esp station has IP
				{
					/*esp_aux_calc_station_ip(ac_ip_check_result);*/
					esp_response(u8connection_ID, ac_client_IP, esp_station_IP);
					timer_delay_ms(3000);
					esp_is_connected = true;
					status_led_mode = connected_to_ap;
					//now it's possible to turn AP off if this is the setting in eeprom
					if(eeprom_read_byte(EEL_ADDR_AP_ALWAYS_ON) == ESP_SYM_AP_NOT_ALWAYS_ON)
					{
						u8work_int = esp_ap_control(ESP_SYM_AP_OFF);
						if (u8work_int != ESP_RETURN_AP_OFF)
						{
							uart_send_string(ERROR_ESP_STATE_JOIN_NEW_NETWORK_ESP_RETURN_CONNECT_SUCCESS_FailedAPSetting);
						}
					}
					u8connection_retry_count = 0;
					u8esp_current_state = ESP_STATE_WAIT_DATA;
					eeprom_save_wifi_credentials(esp_wifi_credentials);
				}
				else if (ESP_RETURN_NOT_CONNECTED == u8work_int) //esp station didn't get an IP
				{
					//try to join AP again
					if(u8connection_retry_count < ESP_CONFIG_CONNECTION_MAX_RETRY)
					{
						u8connection_retry_count++;
						u8esp_current_state = ESP_STATE_JOIN_NEW_NETWORK;
					}
					else
					{
						esp_response(u8connection_ID, ac_client_IP, "Could not connect");
						timer_delay_ms(2000);
						u8connection_retry_count = 0;
						u8esp_current_state = ESP_STATE_START_AP;
						}
				}
				else 
				{
					u8connection_retry_count = 0; 
					uart_send_string("ERROR_ESP_STATE_JOIN_NEW_NETWORK_ESP_RETURN_CONNECT_SUCCESS_UndefinedIPStatus"); 
				}
			}
			// if esp failed in connecting to ssid and pass provided, go back to receive ssid and pass state
			else if(ESP_RETURN_CONNECT_FAILED == u8work_int)
			{
				if(u8connection_retry_count < ESP_CONFIG_CONNECTION_MAX_RETRY)
				{
					u8connection_retry_count++;
					u8esp_current_state = ESP_STATE_JOIN_NEW_NETWORK;
				}
				else 
				{
					esp_response(u8connection_ID, ac_client_IP, "Could not connect");
					timer_delay_ms(2000);
					u8connection_retry_count = 0;
					u8esp_current_state = ESP_STATE_START_AP;
				}
			}
			else 
			{
				u8connection_retry_count = 0;
				uart_send_string("ERROR_ESP_STATE_JOIN_NEW_NETWORK_UndefinedConnectionStatus");
			}
			uart_flush();
		break;
		default:
			//nothing nothing
		break;
		memset(ac_client_IP, 0, BUFFER_SIZE_IP_STRING);
	}
}

