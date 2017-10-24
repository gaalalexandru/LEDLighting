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
#include "usart_handler.h"
#include "configuration.h"

//Pin mapping for wifi module reset (RST_ESP) and enable (CH_PD), pins have to be digital output
//CH_PD: Chip enable. Keep it on high (3.3V) for normal operation
//RST_ESP: Reset. Keep it on high (3.3V) for normal operation. Put it on 0V to reset the chip.
//RST_ESP mapped to MOSI programing pin PB3
//CH_PD mapped MISO programing pin PB4
#define RST_ESP_DIR	DDRB |= (1 << PIN3)
#define	CH_PD_DIR	DDRB |= (1 << PIN4)
#define RST_ESP_SET(x)	PORTB |= ((x) << PIN3)
#define	CH_PD_SET(x)	PORTB |= ((x) << PIN4)  

// get_querry auxiliary function sends a query command to the wifi module
// returns the query response at the end
// Use with caution and only with query commands
// Function gets all characters from buffer and stores it in cmdResponse array.
// At the end saves the cmdResponse to response buffer
static void get_querry(char *cmd, char *response){
	char	cmdResponse[100];
	uint8_t index = 0;
	
	USART_OutString(cmd);
	while(UARTRxBytesAvail())
	{
		cmdResponse[index] = UARTgetc();
		if((cmdResponse[index] == '\r') || (cmdResponse[index] == '\n') ||
		(cmdResponse[index] == 0x1b) || (cmdResponse[index] == 0x0a) ||
		(cmdResponse[index] == 0x0d))
		{
			//do noting
		}
		else
		{
			index++;
		}
	}
	strcpy(response,cmdResponse);
}

// send_command auxiliary function repeats the sent commend until GSM module responses with OK
// Use with caution as not all AT commands have "OK" as good response
// Function gets all characters from buffer and stores it in cmdResponse array.
// At the end a search is performed for "OK" to check if the command was accepted
// If all OK, it saves the cmdResponse to response buffer
static void send_command(char *cmd, char *response){
	uint8_t cmdDone = 0;
	char	cmdResponse[100];
	uint8_t index = 0;
	do {
		USART_OutString(cmd);
		while(UARTRxBytesAvail())
		{
			cmdResponse[index] = UARTgetc();
			if((cmdResponse[index] == '\r') || (cmdResponse[index] == '\n') ||
			(cmdResponse[index] == 0x1b) || (cmdResponse[index] == 0x0a) ||
			(cmdResponse[index] == 0x0d)) {
				//do noting
			}
			else
			{
				index++;
			}
		}
		if(strstr(cmdResponse,"OK") != '\0' )
		{
			cmdDone = 1;
			strcpy(response,cmdResponse);
		}
	} while (!cmdDone);
}

void wifi_init(void)
{
	uint8_t connection_established = 0;
	char *temp_resp;
	RST_ESP_DIR;
	CH_PD_DIR;
	
	RST_ESP_SET(1);
	RST_ESP_SET(0);
	RST_ESP_SET(1);
	
	CH_PD_SET(1);
	
	timer_delay_ms(5000);
	
	do {
		//Check & Set wifi mode
		get_querry("AT+CWMODE_CUR?",&temp_resp);
		if(strstr(temp_resp,"+CWMODE_CUR:1") == '\0' ) //Check if Station Mode is set
		{
			send_command("AT+CWMODE=1", &temp_resp);  //Set Station mode in flash memory
			//Setting temporary mode might not be necessary
			//send_command("AT+CWMODE_CUR=1", &temp_resp);  //Set Station mode
		}

		//Check & Set access point
		get_querry("AT+CWJAP_CUR?",&temp_resp);
		if(strstr(temp_resp,"No AP") == '\0' ) //Check if there is no connection to any AP
		{
			//Might be necessary to have a very long delay between setting AP and OK received
			//If this function is not working, send command separately, wait with timer1, get response
			send_command(strcat("AT+CWJAP_CUR",WIFI_SSID_PASSWORD),&temp_resp); //Set AP in flash memory
			//Setting temporary mode might not be necessary
			//send_command("AT+CWJAP_CUR"+WIFI_SSID+WIFI_PASSWORD,&temp_resp); //Set AP
		}
		
		//Check the wifi state
		get_querry("AT+CIPSTATUS",&temp_resp);
		//Might an alternative to check if state is not 5, meaning NOT connect to an AP
		//if(strstr(temp_resp,"STATUS:5") != '\0' )  // True if string is found in temp_resp
		if(strstr(temp_resp,"STATUS:2") == '\0' ) //Check if ESP connected to an AP and its IP is obtained
		{
			connection_established = 0;  //Not connected
		}
		else
		{
			connection_established = 1;
		}		
	} while(!connection_established);

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

