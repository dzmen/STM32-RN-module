/******************************************************************************
 * File           : RN2483 Library
 *****************************************************************************/
#ifndef _RN2483_H_
#define _RN2483_H_

#include "stm32f0xx.h"
#include <stdint.h>
#include <string.h>
#include <stddef.h>

//This makes IDE happy. Not important for compile.
#include "stm32f0xx_usart.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

#define RN_JOIN_OTAA_MODE           "mac join otaa"
#define RN_JOIN_ABP_MODE            "mac join abp"

// SYS Commands
#define RN_SYS_SLEEP                "sys sleep "				//Sleep in ms. From 100 to 4294967296
#define RN_SYS_GET_VER              "sys get ver"				//Returns the information on hardware platform, firmware version, release date.
#define RN_SYS_FACTORY_RESET        "sys factoryRESET"			//Reset all configurations to factory default
#define RN_SYS_RESET                "sys reset"				//Reboot module

#define RN_SYS_GET_NVM              "sys get nvm 3"			//Get EEPROM data. Address from 0x300 to 0x3FF
#define RN_GET_NVM_LEN              13							//
#define RN_SYS_SET_NVM              "sys set nvm 3"			//Set EEPROM data. Address from 0x300 to 0x3FF
#define RN_SET_NVM_LEN              18							//
#define RN_SYS_GET_VDD              "sys get vdd"				//Get the mV of the module. From 0 - 3600
#define RN_SYS_GET_HWEUI            "sys get hweui"			//Get the HW eui address of the module
#define RN_SYS_SET_PIN              "sys set pindig"			//Set PIN. GPIO0 - GPIO14, UART_CTS, UART_RTS, TEST0, TEST1. to 0 or 1.

// MAC Commands
#define RN_MAC_GET_APP_EUI          "mac get appeui"			//Get current programmed APP eui
#define RN_MAC_GET_STATUS           "mac get status"			//
#define RN_MAC_RESET_CMD            "mac reset 868"			//Reset all configurations for band 868
#define RN_MAC_TX_CMD               "mac tx "					//Send data to Lora network. cnf (confirmed) or uncnf (unconfirmed), port number from 1 to 223, data
#define RN_MAC_JOIN_CMD             "mac join "				//Join Lora network. OTAA (over-the-air activation) or ABP (activation by personalization)
#define RN_MAC_SAVE_CMD             "mac save"					//Save the configuration (band, deveui, appeui, appkey, nwkskey, appskey, devaddr, ch).
#define RN_MAC_PAUSE_CMD            "mac pause"				//Pause LoraWan stack in ms (must be done if you want to change a config while you are already connected to Lora network). From 0 to 4294967295
#define RN_MAC_RESUME_CMD           "mac resume"				//Resume LoraWan stack

#define RN_MAC_SET_DEV_ADDR         "mac set devaddr "			//Set device address. from 00000000 to FFFFFFFF
#define RN_MAC_SET_DEV_EUI          "mac set deveui "			//Set device identifier. 8-byte HEX
#define RN_MAC_SET_APP_EUI          "mac set appeui "
#define RN_MAC_SET_NWK_SESS_KEY     "mac set nwkskey "
#define RN_MAC_SET_APP_SESS_KEY     "mac set appskey "
#define RN_MAC_SET_APP_KEY          "mac set appkey "
#define RN_MAC_GET_STATUS           "mac get status"
#define RN_MAC_SET_DATARATE         "mac set dr "
#define RN_MAC_SET_ADR_ON_CMD       "mac set adr on"
#define RN_MAC_SET_ADR_OFF_CMD      "mac set adr off"
#define RN_MAC_SET_AR_ON_CMD        "mac set ar on"
#define RN_MAC_SET_AR_OFF_CMD       "mac set ar off"

// Radio command
#define RN_RADIO_GET_MODE           "radio get mod"
#define RN_RADIO_SET_MODE           "radio set mod "
#define RN_RADIO_MODE_LEN           18
#define RN_RADIO_SET_PWR            "radio set pwr "
#define RN_RADIO_GET_PWR            "radio get pwr"
#define RN_RADIO_SET_SYNC           "radio set sync "
#define RN_LORA_MODE                "lora"
#define RN_FSK_MODE                 "fsk"

// ----------------------------------------------------------------------------
// Global variables
// ----------------------------------------------------------------------------

typedef enum { false, true } bool;
typedef enum {rnStartup, rnReset, rnSetInfo, rnJoinMethod, rnCheckAbp, rnWaitAbp, rnCheckOtaa, rnWaitOtaa, rnJoined, rnNeedKeys, rnComFailure} RN_JOIN_STATUS;
typedef enum {rnJoining, rnRunning } RN_SYSTEM_STATUS;

static RN_JOIN_STATUS activeJoinState = rnStartup;
static RN_SYSTEM_STATUS activeSystemState = rnJoining;

static uint8_t responseBuffer[64];
static uint8_t responseBufferIndex = 0;
static bool    responseReady = false;

static uint8_t responsePortNum[3];
static uint8_t responseId[16];
static uint8_t responseData[32];

//RN settings
#define RN_RETRY_ATTEMPTS  10
static uint8_t joiningAttempts = 0;

//RN settings OTAA connection
static char *appEui = "0000000000000000";
static char *appKey = "00000000000000000000000000000000";
//RN settings ABP connection
static char *devAddr = "12A00000";
static char *nwkSKey = "00000000000000000000000000000000";
static char *appSKey = "00000000000000000000000000000000";

static uint8_t appJoin = 0; //0 = ABP, 1 = OTAA

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
void  USART_Setup(void);
void  SendCommand(char *str);
void  USART_Send(uint16_t data);
void  SendSettingCommand(char *string, char *data);
void  SendDataCommand(char *string, uint8_t*, uint8_t);
bool  ReceiveData(char *string);
void  RN_Handler(void);
void  RN_Receiver(void);
bool  CheckBufferReady(void);
bool  RN_Join(bool msgReady);
void  RN_Running(bool msgReady);
void  Delay(const int d);

#endif /* _HELPER_H_ */
