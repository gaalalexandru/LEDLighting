/*
 * errors.h
 *
 * Created: 12/15/2018 5:39:19 PM
 *  Author: alexandru.gaal
 */ 

#ifndef ERRORS_H_
#define ERRORS_H_

/*
 * Error codes	|	Error Description and / or location
 * ----------------------------------------------------
 * ER_S10	| State: ESP_STATE_HW_INIT, Function esp_hw_init(), Undefined AP configuration stored in EEPROM
 * ER_S11	| State: ESP_STATE_HW_INIT, Function esp_hw_init(), Failed to synchronize ESP8266 with ATMEGA8
 * ER_S11	| State: ESP_STATE_HW_INIT, Function esp_hw_init(), Failed to set CIPMUX = 1
 * ER_S31	| State: ESP_STATE_START_TCP_SERVER, Failed to stop old server (CIPSERVER = 0)
 * ER_S32	| State: ESP_STATE_START_TCP_SERVER, Failed to start tcp server (CIPSERVER = 1)
 * ER_S33	| State: ESP_STATE_START_TCP_SERVER, Failed to request additional info (CIPDINFO = 1)
 * ER_S34	| State: ESP_STATE_START_TCP_SERVER, Failed to request server timeout (CIPSTO = 60)
 * ER_S51	| State: ESP_STATE_JOIN_NEW_NETWORK, Branch: ESP_RETURN_CONNECT_SUCCESS, Undefined IP status
 * ER_S52	| State: ESP_STATE_JOIN_NEW_NETWORK, Branch: ESP_RETURN_CONNECT_SUCCESS, Failed AP Setting
 * ER_S55	| State: ESP_STATE_JOIN_NEW_NETWORK, Undefined Connection Status
 * ER_A01	| AP Control: Undefined mode requested	
 * ER_A02	| AP Control: Failed AP setting
 * ER_E01	| EEPROM: Not enough space for WiFi credentials (@save)
 * ER_E02	| EEPROM: Not enough space for WiFi credentials (@load)
 * ER_		| 
 */
 
#define ERROR_ESP_STATE_HW_INIT_UndefinedAPConfigStored									"ER_S10"
#define ERROR_ESP_STATE_HW_INIT_FailedSync												"ER_S11"
#define ERROR_ESP_STATE_HW_INIT_FailedSetCipmux											"ER_S12"
#define ERROR_ESP_STATE_START_TCP_SERVER_FailedOldServerStop							"ER_S31"
#define ERROR_ESP_STATE_START_TCP_SERVER_FailedServerStart								"ER_S32"
#define ERROR_ESP_STATE_START_TCP_SERVER_FailedAdditionalInfoRequest					"ER_S33"
#define ERROR_ESP_STATE_START_TCP_SERVER_FailedSterverTimeout							"ER_S34"
#define ERROR_ESP_STATE_JOIN_NEW_NETWORK_ESP_RETURN_CONNECT_SUCCESS_UndefinedIPStatus	"ER_S51"
#define ERROR_ESP_STATE_JOIN_NEW_NETWORK_ESP_RETURN_CONNECT_SUCCESS_FailedAPSetting		"ER_S52"
#define ERROR_ESP_STATE_JOIN_NEW_NETWORK_UndefinedConnectionStatus						"ER_S55"
#define ERROR_ESP_AP_CONTROL_UndefinedAPModeRequested									"ER_A01"
#define ERROR_ESP_AP_CONTROL_FailedAPSetting											"ER_A02"
#define ERROR_EEPROM_SaveCredentialsOverflow											"ER_E01"
#define ERROR_EEPROM_LoadCredentialsOverflow											"ER_E02"

#endif /* ERRORS_H_ */