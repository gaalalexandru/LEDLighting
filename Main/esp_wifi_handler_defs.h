/*
 * esp_wifi_handler_defs.h
 *
 * Created: 12/27/2018 11:55:35 PM
 *  Author: alexandru.gaal
 */ 


#ifndef ESP_WIFI_HANDLER_DEFS_H_
#define ESP_WIFI_HANDLER_DEFS_H_

//esp_state_machine defined states
#define ESP_STATE_HW_INIT			(0)
#define ESP_STATE_CHECK_CONNECTION	(1)
#define	ESP_STATE_START_AP			(2)
#define ESP_STATE_START_TCP_SERVER	(3)
#define ESP_STATE_WAIT_DATA			(4)
#define ESP_STATE_JOIN_NEW_NETWORK	(5)

//esp_wifi_handler defined return codes
#define ESP_RETURN_NDEF				(0xFF)
#define ESP_RETURN_ERROR			(0)
#define ESP_RETURN_OK				(1)
#define ESP_RETURN_NOT_CONNECTED	(2)
#define ESP_RETURN_CONNECTED		(3)
#define ESP_RETURN_AP_ON			(4)
#define ESP_RETURN_AP_OFF			(5)
#define ESP_RETURN_CONNECT_FAILED	(6)
#define ESP_RETURN_CONNECT_SUCCESS	(7)

//esp_wifi_handler defined commands
#define ESP_CMD_GET_STA_IP				('E')
#define ESP_CMD_SET_AUTOCONNECT			('F')
#define ESP_CMD_SET_DEFAULT_PWM			('G')
#define ESP_CMD_SET_STARTUP_ANIM		('H')
#define ESP_CMD_SET_NO_NETWORK_ANIM		('I')
#define ESP_CMD_SET_NO_NETWORK_POWER	('J')
#define ESP_CMD_GET_DEVICE_ID			('K')
#define ESP_CMD_SET_DEVICE_ID			('L')
#define ESP_CMD_GET_DEVICE_SETTINGS		('M')
#define ESP_CMD_SET_AP_ALWAYS_ON		('N')
#define ESP_CMD_SET_AP_ON_OFF			('O')
#define ESP_CMD_SET_BYTE_EEPROM			('P')
#define ESP_CMD_GET_BYTE_EEPROM			('Q')
#define ESP_CMD_RESET_EEPROM			('R')
#define ESP_CMD_RESET_SYSTEM			('X')

//esp_wifi_handler defined symbols
#define ESP_SYM_DATA_IS_PWM_CH		('$')
#define ESP_SYM_DATA_IS_PWM_DC		('#')
#define ESP_SYM_DATA_IS_CMD			('#')
#define ESP_SYM_DATA_IS_SSID		('@')
//#define ESP_SYM_DATA_IS_RESET		('!')  //use X command instead
#define ESP_SYM_AP_OFF				(0x30)
#define ESP_SYM_AP_ON				(0x31)
#define ESP_SYM_AUTOCONNECT_OFF		(0x30)
#define ESP_SYM_AUTOCONNECT_ON		(0x31)
#define ESP_SYM_AP_NOT_ALWAYS_ON	(0x30)
#define ESP_SYM_AP_ALWAYS_ON		(0x31)

#define BUFFER_SIZE_GENERIC_WORK_STRING		(32)
#define BUFFER_SIZE_WIFI_CREDENTIALS_STRING	(40)
#define BUFFER_SIZE_SERIAL_RESULT			(101)
#define BUFFER_SIZE_IP_STRING				(15)
#define true  1
#define false 0


#endif /* ESP_WIFI_HANDLER_DEFS_H_ */