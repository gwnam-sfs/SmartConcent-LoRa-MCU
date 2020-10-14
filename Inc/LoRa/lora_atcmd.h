/*******************************************************************************
 * @file    atcmd.h
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    27-February-2017
 * @brief   Header for driver atcmd.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MDM32L07X01_AT_CMD__
#define __MDM32L07X01_AT_CMD__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32f4xx_hal.h"

/* Exported types ------------------------------------------------------------*/

#define DATA_RX_MAX_BUFF_SIZE    65       /*Max size of the received buffer*/
                                          /*to optimize we can match with device key sizeof*/
  
#define DATA_TX_MAX_BUFF_SIZE    78       /*Max size of the transmit buffer*/
                                          /*it is the worst-case when sending*/
                                          /*a max payload equal to 64 bytes*/

#define TEST_65BYTES "12345678901234567890123456789012345678901234567890123456789012345"
#define TEST_66BYTES "123456789012345678901234567890123456789012345678901234567890123456"

#define TEST_RESP_SCANREQ "02xxF844455631a1b2c3d4e5f62c44455632a1b2c3d4e5f72c44455633a1b2c3d4e5f82c44455634a1b2c3d4e5f92c44455635a1b2c3d4e5faxx03"

#define TEST_NAME_DEVICE1 "44657631"
#define TEST_NAME_DEVICE2 "44657632"
#define TEST_NAME_DEVICE3 "44657633"
#define TEST_NAME_DEVICE4 "44657634"
#define TEST_NAME_DEVICE5 "44657635"

#define TEST_MAC_DEVICE1 "a1b2c3d4e5f6"
#define TEST_MAC_DEVICE2 "a1b2c3d4e5f7"
#define TEST_MAC_DEVICE3 "a1b2c3d4e5f8"
#define TEST_MAC_DEVICE4 "a1b2c3d4e5f9"
#define TEST_MAC_DEVICE5 "a1b2c3d4e5fa"

#define OTB

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(format, arr...) printf(format, arr)
#define DEBUGM_PRINTF(format) printf(format)
#else
#define DEBUG_PRINTF
#define DEBUGM_PRINTF
#endif

#define on 1
#define off 0
#define ON 1
#define OFF 0

typedef enum ATGroup
{
  AT_CTRL = 0,
  AT_SET,
  AT_GET,
  AT_TEST,
} ATGroup_t;

typedef enum Marker_s
{
  CTRL_MARKER = 0,
  SET_MARKER,
  GET_MARKER,
} Marker_t;

/****************************************************************************/
/*here we have to include a list of AT cmd by the way of #include<file>     */
/*this file will be preprocessed for enum ATCmd, enum eATerror and AT marker*/
/*define                                                                    */
/****************************************************************************/


#define  AT_ERROR_INDEX
#define  AT_CMD_INDEX
#define  AT_CMD_MARKER
#include "atcmd_modem.h"    /*to include B-L072Z-LRWAN1 specific string AT cmd definition*/

/*type definition for SEND command*/
typedef struct sSendDataString
{
    char *Buffer;
    uint8_t Port;    
}sSendDataString_t;          

/*type definition for return code analysis*/
typedef  char* ATEerrorStr_t;

typedef struct RetCode_s{
  ATEerrorStr_t RetCodeStr;
  int SizeRetCodeStr;
  ATEerror_t RetCode;
} ATE_RetCode_t;

/*type definition for AT cmd format identification*/
typedef enum Fmt {
  FORMAT_VOID_PARAM,
  FORMAT_8_02X_PARAM,
  FORMAT_16_02X_PARAM,
  FORMAT_32_02X_PARAM,
  FORMAT_32_D_PARAM,
  FORMAT_8_D_PARAM,
  FORMAT_8_C_PARAM,
  FORMAT_PLAIN_TEXT,
  FORMAT_BINARY_TEXT  
}  Fmt_t;  

struct flags {
	uint8_t loraStart : 1;			// Status is Start 		No reset command new start check.  ex) Pseudo Join
	uint8_t loraJoin : 1;			// Status is Join		0: not join, 1: joined
	uint8_t loraReset : 1;			// Reset
	uint8_t loraTransmit : 1;		// Tx Interval check  	0: lock, 1: unlock
	uint8_t loraInputClass : 1;		// Auto input class for Join
	uint8_t loraCycleUplink : 1;	// Send cycle uplink for OTB Test
	uint8_t loraRecvAck : 1;		// Receive Acknowledge Status [TxR:?]
	uint8_t loraMGMTresp : 1;		// Temp flag
	uint8_t sendCmd : 1;
	uint8_t respFromLora : 1;
}flag;

struct mgmtCmdParse {

	uint8_t setPower : 1;
	uint8_t checkPowerValue : 1;
	uint8_t checkBleDevice : 1;
	uint8_t reqWatt : 1;
	uint8_t reqVolt : 1;
	uint8_t reqAmpere : 1;
}mgmtCmd;

typedef uint8_t strHexDest_t[20];

uint16_t at_cmd_vprintf(const char *format, ...);

/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* AT printf */
#define AT_VPRINTF(...)    at_cmd_vprintf(__VA_ARGS__)


/* Exported functions ------------------------------------------------------- */

/******************************************************************************
 * @brief  Configures modem UART interface.
 * @param  None
 * @retval AT_OK in case of success
 * @retval AT_UART_LINK_ERROR in case of failure
*****************************************************************************/
ATEerror_t Modem_IO_Init( void ) ;

/******************************************************************************
 * @brief  Deinitialise modem UART interface.
 * @param  None
 * @retval None
*****************************************************************************/
void Modem_IO_DeInit( void ) ;

/******************************************************************************
 * @brief  Handle the AT cmd following their Groupp type
 * @param  at_group AT group [control, set , get)
 *         Cmd AT command
 *         pdata pointer to the IN/OUT buffer
 * @retval module status
 *****************************************************************************/
ATEerror_t Modem_AT_Cmd(ATGroup_t at_group, ATCmd_t Cmd, void *pdata );

int char2hex(uint8_t s);

int str_to_hex(strHexDest_t dest, uint8_t *p, int size);


void lora_Init();

int get_DevEUI();

int set_DevEUI(char * packet);

int get_AppEUI();

int set_AppEUI(char * packet);

int get_AppKEY();

int set_AppKEY(char * packet);

int get_ConfirmMode();

int set_ConfirmMode(char param);

int get_ClassType();

int set_ClassType(char param);

int get_AutoDR();

int set_AutoDR(char param);

int get_Version();

int get_LibVersion();

int get_JoinStatus();

int run_Reset();

int get_lora_message (char *packet);

int lora_message_analysis (char * dataFromLora, int len);

void lora_command_process();

int mgmtCmd_analysis(char * dataFromLora, int len);

int send_lora_message(char * message);

int get_console_message(char * packet);

int console_message_analysis (char * dataFromConsole, int len);

void send_uplink_message(char * packet, int size, int fport);

void sendb_uplink_message(char * packet, int size, int fport);

int buildReport(char * result, char cmd, char * tgDev, int value);

int buildBleDeviceReport(char * result, char cmd);

void lora_OTB_cmd_test(char * dataFromConsole, int len);

void lora_OTB_CyUp_test();




#ifdef __cplusplus
}
#endif

#endif /* __MDM32L07X01_AT_CMD__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
