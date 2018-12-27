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
 * ERR_S00	| State: ESP_STATE_HW_INIT, Function esp_hw_init(), Undefined AP configuration stored in EEPROM
 * ERR_S01	| State: ESP_STATE_HW_INIT, Function esp_hw_init(), Failed to synchronize ESP8266 with ATMEGA8
 * ERR_S02	| State: ESP_STATE_HW_INIT, Function esp_hw_init(), Failed to set CIPMUX = 1
 * ERR_S10	| State: ESP_STATE_CHECK_CONNECTION, Function esp_check_connection returned unexpected result
 * ERR_S20	| State: ESP_STATE_START_AP, Failed to start AP
 * ERR_S31	| State: ESP_STATE_START_TCP_SERVER, Failed to stop old server (CIPSERVER = 0)
 * ERR_S32	| State: ESP_STATE_START_TCP_SERVER, Failed to start tcp server (CIPSERVER = 1)
 * ERR_S33	| State: ESP_STATE_START_TCP_SERVER, Failed to request additional info (CIPDINFO = 1)
 * ERR_S34	| State: ESP_STATE_START_TCP_SERVER, Failed to request server timeout (CIPSTO = 60)
 * ERR_S51	| State: ESP_STATE_JOIN_NEW_NETWORK, Branch: ESP_RETURN_CONNECT_SUCCESS, Undefined IP status
 * ERR_S52	| State: ESP_STATE_JOIN_NEW_NETWORK, Branch: ESP_RETURN_CONNECT_SUCCESS, Failed AP Setting
 * ERR_S55	| State: ESP_STATE_JOIN_NEW_NETWORK, Undefined Connection Status
 * ERR_A01	| AP Control: Undefined mode requested	
 * ERR_A02	| AP Control: Failed AP setting
 * ERR_E01	| EEPROM: Not enough space for WiFi credentials (@save)
 * ERR_E02	| EEPROM: Not enough space for WiFi credentials (@load)
 * ERR_A05	| ANIMATION: Undefined startup animation coded in EEPROM
 * ERR_A06  | ANIMATION: Undefined no network connection notification animation coded in EEPROM
 */
 
#define ERROR_ESP_STATE_HW_INIT_UndefinedAPConfigStored									"ERR_S00"
#define ERROR_ESP_STATE_HW_INIT_FailedSync												"ERR_S01"
#define ERROR_ESP_STATE_HW_INIT_FailedSetCipmux											"ERR_S02"
#define ERROR_ESP_STATE_CHECK_CONNECTION_UnknownCheckConnResult							"ERR_S10"
#define ERRPR_ESP_STATE_START_AP_FailedStartAP											"ERR_S20"
#define ERROR_ESP_STATE_START_TCP_SERVER_FailedOldServerStop							"ERR_S31"
#define ERROR_ESP_STATE_START_TCP_SERVER_FailedServerStart								"ERR_S32"
#define ERROR_ESP_STATE_START_TCP_SERVER_FailedAdditionalInfoRequest					"ERR_S33"
#define ERROR_ESP_STATE_START_TCP_SERVER_FailedSterverTimeout							"ERR_S34"
#define ERROR_ESP_STATE_JOIN_NEW_NETWORK_ESP_RETURN_CONNECT_SUCCESS_UndefinedIPStatus	"ERR_S51"
#define ERROR_ESP_STATE_JOIN_NEW_NETWORK_ESP_RETURN_CONNECT_SUCCESS_FailedAPSetting		"ERR_S52"
#define ERROR_ESP_STATE_JOIN_NEW_NETWORK_UndefinedConnectionStatus						"ERR_S55"
#define ERROR_ESP_AP_CONTROL_UndefinedAPModeRequested									"ERR_A01"
#define ERROR_ESP_AP_CONTROL_FailedAPSetting											"ERR_A02"
#define ERROR_ANIMATION_UndefinedStartupAnimationCoded									"ERR_A05"
#define ERROR_ANIMATION_UndefinedNoNetAnimationCoded									"ERR_A06"
#define ERROR_EEPROM_SaveCredentialsOverflow											"ERR_E01"
#define ERROR_EEPROM_LoadCredentialsOverflow											"ERR_E02"

#endif /* ERRORS_H_ */