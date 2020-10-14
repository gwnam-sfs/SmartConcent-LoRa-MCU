/**
  ******************************************************************************
  * @file    atcmd.c
  * @author  MCD Application Team
  * @brief   at command API
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <stdarg.h>
#include "LoRa\tiny_vsnprintf.h"
#include "LoRa\lora_atcmd.h"

//#include "debug.h"


/* External variables --------------------------------------------------------*/


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/

extern char set_apptimer(int id, int expires);
extern char chk_apptimer(int id);
extern int testCount;

static char *chkMsg[] = {
  "Start Lora",
  "RESET.",
  "OnJoined - 2",
  "OnJoined - 3",
  "TxDone",
  "Input Class",
  "TxR:1",
  "TxR:0",
  "MGMT=",
  "OK"
};

char LoRa_AT_Cmd_Buff[DATA_TX_MAX_BUFF_SIZE];    /* Buffer used for AT cmd transmission */

//static uint16_t Offset = 0;   /*write position needed for sendb command*/

//static uint8_t aRxBuffer[5];  /* Buffer used for Rx input character */

//static char response[DATA_RX_MAX_BUFF_SIZE];
/*has to be the largest of the response*/
/*not only for return code but also for*/
/*return value: exemple KEY*/

char storeDevEUI[8] = {0x70, 0xb3, 0xd5, 0x67, 0x60, 0x00, 0x58, 0x20};
char storeAppEUI[8] = {0x99, 0x99, 0x99, 0x10, 0x00, 0x00, 0x04, 0x68};
char storeAppKEY[16] = {0xf2, 0xbe, 0x3d, 0x0c, 0xb4, 0x87, 0x4b, 0xab, 0x98, 0xd9, 0x30, 0xfa, 0x3c, 0x24, 0x4e, 0xa4};

char macAddress[13] = {0,};

/****************************************************************************/
/*here we have to include a list of AT cmd by the way of #include<file>     */
/*this file will be preprocessed for CmdTab and ATE_RetCode definition      */
/****************************************************************************/

//#undef    __ATCMD_MODEM_H__    /*to avoid recursive include*/
#define   AT_CMD_STRING
#define   AT_ERROR_STRING
#undef    AT_CMD_INDEX
#undef    AT_ERROR_INDEX
#include <stdio.h>
#include "main.h"
#include "usart.h"
#include "UartRingbuffer.h"
#include "LoRa/atcmd_modem.h"   /*to include B-L072Z-LRWAN1 specific string AT cmd definition*/


/* private functions ------------------------------------------------------- */


/* Exported functions ------------------------------------------------------- */


/******************************************************************************
 * @brief  Handle the AT cmd following their Groupp type
 * @param  at_group AT group [control, set , get)
 *         Cmd AT command
 *         pdata pointer to the IN/OUT buffer
 * @retval module status
 *****************************************************************************/


/******************************************************************************
 * @brief  format the cmd in order to be send
 * @param  Cmd AT command
 *         ptr generic pointer to the IN/OUT buffer
 *         Marker to discriminate the Set from the Get
 * @retval length of the formated frame to be send
 *****************************************************************************/


/******************************************************************************
  * @brief This function sends an AT cmd to the slave device
  * @param len: length of the AT cmd to be sent
  * @retval HAL return code
******************************************************************************/


/******************************************************************************
  * @brief This function receives response from the slave device
  * @param pdata: pointer to the value returned by the slave
  * @retval return code coming from slave
******************************************************************************/


/******************************************************************************
  * @brief This function does analysis of the response received by the device
  * @param response: pointer to the received response
  * @retval ATEerror_t error type
******************************************************************************/


/******************************************************************************
  * @brief format the AT frame to be sent to the modem (slave)
  * @param pointer to the format string
  * @retval len of the string to be sent
******************************************************************************/



/* Private functions ---------------------------------------------------------*/

void lora_Init() {

	flag.loraStart = 0;
	flag.loraJoin = 0;
	flag.loraReset = 0;
	flag.loraTransmit = 1;
	flag.loraInputClass = 0;
	flag.loraCycleUplink = 0;
	flag.loraRecvAck = 0;
	flag.loraMGMTresp = 0;
	flag.sendCmd = 0;
	flag.respFromLora = 0;
	mgmtCmd.reqWatt = 0;
	mgmtCmd.reqVolt = 0;
	mgmtCmd.reqAmpere = 0;
	mgmtCmd.checkBleDevice = 0;

}


/*
 * char buffer 를 인수로 넘겨주면
 * LoRa UART 링버퍼에서 \n을 만날 때까지 message를 버퍼에 넣는다.
 * message 길이를 리턴한다.
 * 3초 동안 통신이 안 되면 -1을 리턴한다.
 */
int get_lora_message (char *packet) {
	int ret = 0;
	char r = 0;


	// set timeout 5sec and start
	set_apptimer(5, 3000);

	while (1)
	{
		if (chk_apptimer(5))		/* timeout check */
			return -1;
		if (!isgetc_from_lora())	/* nothing Ringbuffer continue loop */
			continue;

		r = lora_getc(); 	/* Read Ringbuffer */
		*packet++ = r; 		/* into Packet */
		ret++;				/* size count */

		if (r == '\n')		/* '\n' packet end */
			return ret;		/* size */

	}//while(1)
}

int lora_message_analysis (char * dataFromLora, int len) {
	int ret;
	char tempBuffer[150];
	/*
	 * Start LoRa
	 */
	if (memcmp(chkMsg[0], dataFromLora, sizeof(chkMsg[0]) - 1) == 0){
		flag.loraStart = 1; //
		set_apptimer(6, 3000); 	/* Action 3 sec later */
		ret = 1;
	}

	/*
	 * [RESET.] SKT_MSG_REBOOT receive -> response -> received ACK
	 */
	else if (memcmp(chkMsg[1], dataFromLora, sizeof(chkMsg[1]) - 1) == 0){
		flag.loraReset = 1;
		set_apptimer(7, 2000);
		ret = 2;
	}

	/*
	 *  OnJoined - 1 Just Pseudo joined
	 *  OnJoined - 2 Just Real joined 			already get Real_key
	 *  OnJoined - 3 Pseudo join -> Real join 	new Real_key setting
	 */
	else if (memcmp(chkMsg[2], dataFromLora, sizeof(chkMsg[2]) - 1) == 0 ||
			 memcmp(chkMsg[3], dataFromLora, sizeof(chkMsg[3]) - 1) == 0) {
		flag.loraJoin = 1;

		/* Unable Tx for 15 sec after join */
		flag.loraTransmit = 0;
		set_apptimer(8, 15000);
		DEBUGM_PRINTF("[GWD] Tx OFF\r\n");

		/* Cycle uplink start (OTB test) */
		flag.loraCycleUplink = 1;
		flag.loraRecvAck = 1;
		testCount = 0;
		set_apptimer(9, 13000); /* First Tx begin 13 sec later : cycle_uplink */
		printf("Cycle Uplink Test Begin\r\n");
		ret = 3;
	}

	/*
	 * Next Tx 10 sec later after last Tx
	 * [TxDone] message -> lock Tx -> 10 sec -> unlock Tx
	 */
	else if (memcmp(chkMsg[4], dataFromLora, sizeof(chkMsg[4]) - 1) == 0){
		flag.loraTransmit = 0;
		set_apptimer(8, 13000);
		if (flag.loraJoin == 1) DEBUGM_PRINTF("[GWD] Tx OFF\r\n");
		ret = 4;
	}

	/*
	 * [Input Class] message -> Tx to lora module AT+CLASS=C
	 */
	else if (memcmp(chkMsg[5], dataFromLora, sizeof(chkMsg[5]) - 1) == 0){
		flag.loraInputClass = 1;
		ret = 5;
	}

	/*
	 * TxR:1 TxResult OK
	 */
	else if (memcmp(chkMsg[6], dataFromLora,  sizeof(chkMsg[6]) - 1) == 0) {
		flag.loraRecvAck = 1;
		set_apptimer(9, 60000);
		if (flag.loraMGMTresp == 1) set_apptimer(6, 14000);
		DEBUGM_PRINTF("[GWD] Receive Ack - success\r\n");
		ret = 6;
	}

	/*
	 * TxR:0 TxResult fail
	 */
	else if (memcmp(chkMsg[7], dataFromLora, sizeof(chkMsg[7]) - 1) == 0) {
		flag.loraRecvAck = 1;
		set_apptimer(9, 60000);
		if (flag.loraMGMTresp == 1) set_apptimer(6, 17000);
		DEBUGM_PRINTF("[GWD] Receive Ack - fail\r\n");
		ret = 7;
	}

	/*
	 * MGMT=
	 */
	else if (memcmp(chkMsg[8], dataFromLora, sizeof(chkMsg[8]) - 1) == 0) {
		flag.loraMGMTresp = 1;
		memcpy(tempBuffer, dataFromLora, len);
		memset(dataFromLora, 0x00, len);
		memcpy(dataFromLora, tempBuffer + 5, len - 5);

		set_apptimer(6, 14000);
		ret = 8;
		return ret;
	}

	else if (flag.sendCmd == 1 && (dataFromLora[0] != 0x00)) {
		flag.sendCmd = 0;
		return 9;
	}

	else {
		ret = 0;
	}
	memset(dataFromLora, 0x00, len);
	return ret;
}

void lora_command_process() {
	/* Start check */
	if (flag.loraStart == 1 && chk_apptimer(6)) {
		flag.loraStart = 0;
		if (flag.loraJoin == 1) flag.loraJoin = 0;
	}

	/* Reset process */
	if (flag.loraReset == 1 && chk_apptimer(7)) {
		run_Reset();
		//lora_puts("AT+RESET\r\n");		/* send command lora module(Reset) */
		//flag.loraReset = 0;				/* Request Reset finish */
		//flag.loraJoin = 0;				/* Join state is not joined */
		//flag.loraTransmit = 1;			/* Tx available */
	}

	/* First Tx for 10 sec later after join */ /* Next Tx 10 sec later last Tx */
	if (flag.loraTransmit == 0 && chk_apptimer(8)){
		flag.loraTransmit = 1;			/*  Tx available  */
		if (flag.loraJoin == 1) DEBUGM_PRINTF("[GWD] Tx ON\r\n");
	}

	/* Input Class for join */
	if (flag.loraJoin == 0 && flag.loraInputClass == 1 && flag.loraStart == 0){
		//lora_puts("AT+CLASS=C\r\n");	/* send command lora module(Input Class) */
		flag.loraInputClass = 0;				/* Input Class finish */
	}

	if (flag.loraMGMTresp == 1 && chk_apptimer(6)) {
		/* Create Random Value */
		char tmpTxString[17] = {0,};
		uint8_t randNum[5] = {0,};


		/* Tx Power On status & Power value */
		//if (mgmtCmd.setPower == ON) {
		//	for (int i = 0; i < 5; i ++) { randNum[i] = (uint8_t)(rand() % 80); }
		//	sprintf(tmpTxString, "020206%02d%02d%02d%02d%02d", randNum[0], randNum[1], randNum[2], randNum[3], randNum[4]);
		//	sendb_uplink_message(tmpTxString, strlen(tmpTxString), 10);
		//	flag.loraMGMTresp = 0;
		//}

		if (mgmtCmd.reqWatt == 1) {
			char cmd = 0xf0;
			char buildResult[29] = {0,};
			int tempValue = (((rand() % 100) * 1000) + (rand() % 1000));
			if (tempValue % 10 == 0)
				tempValue *= -1;
			buildReport(buildResult, cmd, macAddress, tempValue);
			sendb_uplink_message(buildResult, strlen(buildResult), 19);
			mgmtCmd.reqWatt = 0;
			memset(macAddress, 0x00, 13);
		}

		if (mgmtCmd.checkBleDevice == 1) {
			char cmd = 0xf8;
			char buildResult[108] = {0,};
			buildBleDeviceReport(buildResult, cmd);
			sendb_uplink_message(buildResult, strlen(buildResult), 28);
			mgmtCmd.checkBleDevice = 0;

		}
	}

}

/*
 * mgmt Command analysis
 */
int mgmtCmd_analysis(char * dataFromLora, int len) {
	int ret = 0;
	if (!(*dataFromLora == '0' && *(dataFromLora + 1) == '2')) return -1;
	//if (len != strlen(dataFromLora)) return -1;

	dataFromLora += 4;

	/*  */
	if (*dataFromLora == 'F' || *dataFromLora == 'f') {
		switch (*(dataFromLora + 1)) {
			case '0':
				dataFromLora += 2;

				memcpy(macAddress, dataFromLora, 12);
				macAddress[12] = '\0';
				mgmtCmd.reqWatt = 1;
				ret = 0;
				break;
			case '8':
				dataFromLora += 2;

				mgmtCmd.checkBleDevice = 1;
				ret = 0;
				break;

			default:
				ret = 1;
		}
	}
	return ret;
}


/*
 * Base Function
 * get data message from PC console and store buffer
 * input empty buffer pointer
 *
 * return data size
 */
int get_console_message (char *packet) {
	int ret = 0;		//Return Value : datasize
	char r = 0;			//Get char from Ring_buffer

	/* for check UART time out : 3 sec */
	set_apptimer(5, 3000);

	while (1) {
		/* for 3 sec UART check time out : return -1(error) */
		if (chk_apptimer(5))
			return -1;
		/* if nothing at Ring_buffer : continue loop */
		if (!isgetc_from_console())
			continue;

		r = console_getc(); // 링버퍼에서 읽어오기
		*packet++ = r; 		// 버퍼에 담기
		ret++;

		/* data size return when \n */
		if (r == '\n')
			return ret;
	}
}

int console_message_analysis (char * dataFromConsole, int len) {
	int ret;
	/* "AT+SEND=" is uplink to server */
	if (memcmp("AT+SEND=", dataFromConsole, sizeof("AT+SEND=") - 1) == 0){

		send_lora_message(dataFromConsole);
		ret = 1;
	}
	/* other command send to lora module */
	else {
		lora_puts(dataFromConsole);
		ret = 0;
	}

	memset(dataFromConsole, 0x00 , len);
	return ret;
}

/*
 * send data message to loRa module
 * Input data message buffer pointer
 * Check Join & Transmit interval
 *
 * return void
 */
int send_lora_message(char * message) {
	/* uplink is only possible in the joined state */
	if (flag.loraJoin == 0)
	{
		printf("AT_NO_NETWORK_JOINED_GW\r\n");
		return -1;
	}
	/* Can't send unless it's ready */
	if (flag.loraTransmit != 1)
	{
		printf("AT_BUSY_ERROR_GW\r\n");
		HAL_Delay(100);
		return -2;
	}

	//DEBUG_PRINTF("%s", message);
	lora_puts(message);
	return 0;
}

int buildReport(char * result, char cmd, char * tgDev, int value) {
	char stx = 0x02;
	char etx = 0x03;
	char len = 28;

	switch (cmd) {
		case 0xF0: // 전력량 (float * 1000) -> integer(4bytes) 전송, 앱수신  (integer / 1000) -> float -> display
			sprintf(result, "%02x%02x%02x%s%08x%02x", stx, len, cmd, tgDev, value, etx);
			break;
		case 0xF1: // 전압 (float * 1000) -> integer(4bytes) 전송, 앱수신  (integer / 1000) -> float -> display
			sprintf(result, "%02x%02x%02x%s%08x%02x", stx, len, cmd, tgDev, value, etx);
			break;
		case 0xF2: // 전류 (float * 1000) -> integer(4bytes) 전송, 앱수신  (integer / 1000) -> float -> display
			sprintf(result, "%02x%02x%02x%s%08x%02x", stx, len, cmd, tgDev, value, etx);
			break;
		case 0xF3:
			break;
	}
	return 0;
}

int buildBleDeviceReport(char * result, char cmd) {
	char stx = 0x02;
	char etx = 0x03;
	char len = 54;

	sprintf(result, "%02x%02x%02x%s%s%s%s%s%s%s%s%s%s%02x", stx, len, cmd,
			TEST_NAME_DEVICE1, TEST_MAC_DEVICE1,
			TEST_NAME_DEVICE2, TEST_MAC_DEVICE2,
			TEST_NAME_DEVICE3, TEST_MAC_DEVICE3,
			TEST_NAME_DEVICE4, TEST_MAC_DEVICE4,
			TEST_NAME_DEVICE5, TEST_MAC_DEVICE5, etx);
	return 0;
}


#ifdef OTB
/*
 * Use to OTB test command
 * Input
 * 65 Bytes Send test -> S1
 * 66 Bytes Send test -> S2
 * DEV_TIME_REQ test -> DE
 * LINK_CHK_REQ test -> LC
 * Cycle Uplink test begin -> CB : unused
 * Cycle Uplink test end -> CE
 * Input Class to Join -> IC
 *
 * return void
 */
void lora_OTB_cmd_test(char * dataFromConsole, int len) {
	/* Command to send 65 bytes to server - max payload */
	if (memcmp("S1", dataFromConsole, 2) == 0){
		send_uplink_message(TEST_65BYTES, sizeof(TEST_65BYTES) - 1, 101);
		memset(dataFromConsole, 0x00, len);
		return;
	}
	/* Command to send 66 bytes to server - over payload */
	if (memcmp("S2", dataFromConsole, 2) == 0){
		send_uplink_message(TEST_66BYTES, sizeof(TEST_66BYTES) - 1, 101);
		memset(dataFromConsole, 0x00, len);
		return;
	}
	/* Command send to request device time to server */
	if (memcmp("DE", dataFromConsole, 2) == 0){
		send_lora_message("AT+DEV_TIME_REQ\r\n");
		DEBUGM_PRINTF("[GWD]DEV_TIME_REQ\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}
	/* Command send to request link check to server */
	if (memcmp("LC", dataFromConsole, 2) == 0){
		send_lora_message("AT+LINK_CHK_REQ\r\n");
		DEBUGM_PRINTF("[GWD]LINK_CHK_REQ\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}
	/* Command send to begin cycle uplink  */
	if (memcmp("CB", dataFromConsole, 2) == 0){
		flag.loraCycleUplink = 1;	/* Turn on for begin uplink */
		flag.loraRecvAck = 1;		/* Switch for the first Tx*/
		testCount = 0;				/* test count reset */
		printf("Cycle Uplink Test Begin\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}
	/* Command send to end cycle uplink */
	if (memcmp("CE", dataFromConsole, 2) == 0){
		flag.loraCycleUplink = 0;	/* Turn off for end uplink */
		printf("Cycle Uplink Test End\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}
	/* Command send to input class to join */
	if (memcmp("IC", dataFromConsole, 2) == 0){
		set_ClassType(2);
		DEBUGM_PRINTF("[GWD] AT+CLASS=C\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test get_DevEUI() */
	if (memcmp("GDE", dataFromConsole, 3) == 0){
		get_DevEUI();
		DEBUGM_PRINTF("[GWD] AT+DEUI=?\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test set_DevEUI() */
	if (memcmp("SDE", dataFromConsole, 3) == 0){
		set_DevEUI(storeDevEUI);
		DEBUG_PRINTF("[GWD] AT+#DEV_EUI_SET#=%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
				storeDevEUI[0], storeDevEUI[1], storeDevEUI[2], storeDevEUI[3],
				storeDevEUI[4], storeDevEUI[5], storeDevEUI[6], storeDevEUI[7]);
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test get_AppEUI() */
	if (memcmp("GAE", dataFromConsole, 3) == 0){
		get_AppEUI();
		DEBUGM_PRINTF("[GWD] AT+APPEUI=?\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test set_AppEUI() */
	if (memcmp("SAE", dataFromConsole, 3) == 0){
		set_AppEUI(storeAppEUI);
		DEBUG_PRINTF("[GWD] AT+APPEUI=%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
				storeAppEUI[0], storeAppEUI[1], storeAppEUI[2], storeAppEUI[3],
				storeAppEUI[4], storeAppEUI[5], storeAppEUI[6], storeAppEUI[7]);
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test get_AppKEY() */
	if (memcmp("GAK", dataFromConsole, 3) == 0){
		get_AppKEY();
		DEBUGM_PRINTF("[GWD] AT+APPKEY=?\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test set_AppKEY() */
	if (memcmp("SAK", dataFromConsole, 3) == 0){
		set_AppKEY(storeAppKEY);
		DEBUG_PRINTF("[GWD] AT+APPKEY=%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
				storeAppKEY[0], storeAppKEY[1], storeAppKEY[2], storeAppKEY[3],
				storeAppKEY[4], storeAppKEY[5], storeAppKEY[6], storeAppKEY[7],
				storeAppKEY[8], storeAppKEY[9], storeAppKEY[10], storeAppKEY[11],
				storeAppKEY[12], storeAppKEY[13], storeAppKEY[14], storeAppKEY[15]);
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test get_ConfirmMode() */
	if (memcmp("GCM", dataFromConsole, 3) == 0){
		get_ConfirmMode();
		DEBUGM_PRINTF("[GWD] AT+CFM=?\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test set_ConfirmMode(1) */
	if (memcmp("SCM1", dataFromConsole, 4) == 0){
		set_ConfirmMode(1);
		DEBUGM_PRINTF("[GWD] AT+CFM=1\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test set_ConfirmMode(0) */
	if (memcmp("SCM0", dataFromConsole, 4) == 0){
		set_ConfirmMode(0);
		DEBUGM_PRINTF("[GWD] AT+CFM=0\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test set_ConfirmMode(2) */
	if (memcmp("SCM2", dataFromConsole, 4) == 0){
		set_ConfirmMode(2);
		DEBUGM_PRINTF("[GWD] AT+CFM=2\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test get_ClassType() */
	if (memcmp("GCT", dataFromConsole, 3) == 0){
		get_ClassType();
		DEBUGM_PRINTF("[GWD] AT+CLASS=?\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test set_ClassType(2) */
	if (memcmp("SCT2", dataFromConsole, 4) == 0){
		if (set_ClassType(2) == -1) {
			memset(dataFromConsole, 0x00, len);
			return;
		}
		DEBUGM_PRINTF("[GWD] AT+CLASS=C\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test get_AutoDR() */
	if (memcmp("GAD", dataFromConsole, 3) == 0){
		get_AutoDR();
		DEBUGM_PRINTF("[GWD] AT+ADR=?\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test set_AutoDR(1) */
	if (memcmp("SAD1", dataFromConsole, 4) == 0){
		set_AutoDR(1);
		DEBUGM_PRINTF("[GWD] AT+ADR=1\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test get_Version() */
	if (memcmp("GV", dataFromConsole, 2) == 0){
		get_Version();
		DEBUGM_PRINTF("[GWD] AT+VER=?\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test get_LibVersion() */
	if (memcmp("GLV", dataFromConsole, 3) == 0){
		get_LibVersion();
		DEBUGM_PRINTF("[GWD] AT+LIB_VER=?\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test get_JoinStatus() */
	if (memcmp("GJS", dataFromConsole, 3) == 0){
		get_JoinStatus();
		DEBUGM_PRINTF("[GWD] AT+NJS=?\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}

	/* Function Test get_JoinStatus() */
	if (memcmp("RRS", dataFromConsole, 3) == 0){
		run_Reset();
		DEBUGM_PRINTF("[GWD] AT+RESET\r\n");
		memset(dataFromConsole, 0x00, len);
		return;
	}
}

/*
 * Use to OTB Cycle Uplink test
 * Input
 * Check 60 sec later after receive ACK send test count
 * testCount 1~10000
 * return void
 */
void lora_OTB_CyUp_test() {
	char cycleUplink[6] = {0,};

	if(flag.loraRecvAck == 1 && chk_apptimer(9)){
		testCount++;
		flag.loraRecvAck = 0;
		if (testCount > 10000){
			flag.loraCycleUplink = 0;
			testCount = 0;
			printf("Cycle Uplink Test End\r\n");
			return;
		}

		sprintf(cycleUplink, "%05d", testCount);
		send_uplink_message(cycleUplink, sizeof(cycleUplink) - 1, 100);
	}
}
#endif

/*
 * AT+SEND= uplink function
 */
void send_uplink_message(char *packet, int size, int fport) {
	char uplinkbuffer[80] = {0,};

	sprintf (uplinkbuffer, "AT+SEND=%03d:%s\r\n", fport, packet);

	if (send_lora_message(uplinkbuffer) < 0) {
		return;
	}
	DEBUG_PRINTF("[GWD] %d %s", size, uplinkbuffer);
	return;
}

/*
 * AT+SENDB= uplink function
 */
void sendb_uplink_message(char *packet, int size, int fport) {
	char uplinkbuffer[150] = {0,};

	sprintf (uplinkbuffer, "AT+SENDB=%03d:%s\r\n", fport, packet);

	if (send_lora_message(uplinkbuffer) < 0) {
		return;
	}
	DEBUG_PRINTF("[GWD] %d %s %d\r\n", size, uplinkbuffer, strlen(uplinkbuffer));
	return;
}

/*
 * DevEUI check
 */
int get_DevEUI() {
	char sendLoraModule[12] = {0,};

	sprintf(sendLoraModule, "AT+DEUI=?\r\n");
	lora_puts(sendLoraModule);
	flag.sendCmd = 1;
	return 0;
}

/*
 * DevEUI setting
 */
int set_DevEUI(char * packet) {
	char sendLoraModule[43];

	sprintf(sendLoraModule, "AT+#DEV_EUI_SET#=%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
			packet[0], packet[1], packet[2], packet[3],
			packet[4], packet[5], packet[6], packet[7]);
	flag.sendCmd = 1;
	lora_puts(sendLoraModule);
	return 0;
}

/*
 * AppEUI check
 */
int get_AppEUI() {
	char sendLoraModule[14] = {0,};

	sprintf(sendLoraModule, "AT+APPEUI=?\r\n");
	lora_puts(sendLoraModule);
	flag.sendCmd = 1;
	return 0;
}

/*
 * AppEUI setting
 */
int set_AppEUI(char * packet) {
	char sendLoraModule[36];

	sprintf(sendLoraModule, "AT+APPEUI=%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
			packet[0], packet[1], packet[2], packet[3],
			packet[4], packet[5], packet[6], packet[7]);
	flag.sendCmd = 1;
	lora_puts(sendLoraModule);
	return 0;
}

/*
 * AppKEY check, getting NULL after REAL JOIN
 */
int get_AppKEY() {
	char sendLoraModule[14] = {0,};

	sprintf(sendLoraModule, "AT+APPKEY=?\r\n");
	lora_puts(sendLoraModule);
	flag.sendCmd = 1;
	return 0;
}

/*
 * AppKEY setting
 */
int set_AppKEY(char * packet) {
	char sendLoraModule[60];

	sprintf(sendLoraModule, "AT+APPEUI=%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
			packet[0], packet[1], packet[2], packet[3],
			packet[4], packet[5], packet[6], packet[7],
			packet[8], packet[9], packet[10], packet[11],
			packet[12], packet[13], packet[14], packet[15]);
	flag.sendCmd = 1;
	lora_puts(sendLoraModule);
	return 0;
}

/*
 * Confirm Mode check
 */
int get_ConfirmMode() {
	char sendLoraModule[11] = {0,};

	sprintf(sendLoraModule, "AT+CFM=?\r\n");
	lora_puts(sendLoraModule);
	flag.sendCmd = 1;
	return 0;
}

/*
 * Confirm Mode setting
 */
int set_ConfirmMode(char param) {
	if (param != 1 && param != 0) {
		DEBUGM_PRINTF("[GWD] AT_PARAM_ERROR\r\n");
		return -1;
	}
	char sendLoraModule[11] = {0,};

	sprintf(sendLoraModule, "AT+CFM=%d\r\n", param);
	lora_puts(sendLoraModule);
	flag.sendCmd = 1;
	return 0;
}

/*
 * Class Type check
 */
int get_ClassType() {
	char sendLoraModule[13] = {0,};

	sprintf(sendLoraModule, "AT+CLASS=?\r\n");
	lora_puts(sendLoraModule);
	flag.sendCmd = 1;
	return 0;
}

/*
 * Class Type setting
 * C type = 2, 'C', 'c'
 * A type = 0, 'A', 'a'
 */
int set_ClassType(char param) {
	if (param != 2 && param != 0 && param != 'C' && param != 'A' && param != 'c' && param != 'a') {
		DEBUGM_PRINTF("[GWD] AT_PARAM_ERROR\r\n");
		return -1;
	}
	else if (param == 2 || param == 'c') param = 'C';
	else if (param == 0 || param == 'a') param = 'A';

	char sendLoraModule[13] = {0,};

	sprintf(sendLoraModule, "AT+CLASS=%c\r\n", param);
	lora_puts(sendLoraModule);
	flag.sendCmd = 1;
	return 0;
}

/*
 * AutoDataRate config check
 */
int get_AutoDR() {
	char sendLoraModule[11] = {0,};

	sprintf(sendLoraModule, "AT+ADR=?\r\n");
	lora_puts(sendLoraModule);
	flag.sendCmd = 1;
	return 0;
}

/*
 * AutoDataRate config setting
 */
int set_AutoDR(char param) {
	if (param != 1 && param != 0) {
		DEBUGM_PRINTF("[GWD] AT_PARAM_ERROR\r\n");
		return -1;
	}
	char sendLoraModule[11] = {0,};

	sprintf(sendLoraModule, "AT+ADR=%d\r\n", param);
	lora_puts(sendLoraModule);
	flag.sendCmd = 1;
	return 0;
}

/*
 * Module FirmWare Version check
 */
int get_Version() {
	char sendLoraModule[11] = {0,};

	sprintf(sendLoraModule, "AT+VER=?\r\n");
	lora_puts(sendLoraModule);
	flag.sendCmd = 1;
	return 0;
}

/*
 * LoRaWAN Library Version check
 */
int get_LibVersion() {
	char sendLoraModule[15] = {0,};

	sprintf(sendLoraModule, "AT+LIB_VER=?\r\n");
	lora_puts(sendLoraModule);
	flag.sendCmd = 1;
	return 0;
}

/*
 * Join Status check
 */
int get_JoinStatus() {
	char sendLoraModule[11] = {0,};

	sprintf(sendLoraModule, "AT+NJS=?\r\n");
	lora_puts(sendLoraModule);
	flag.sendCmd = 1;
	return 0;
}

/*
 * RESET run (AT command)
 */
int run_Reset() {
	char sendLoraModule[11] = {0,};

	sprintf(sendLoraModule, "AT+RESET\r\n");
	lora_puts(sendLoraModule);
	lora_Init();
	return 0;
}

int char2hex(uint8_t s)
{
	return ((s>= '0' && s<='9') ? (s-'0') : ((s>= 'a' && s<='f') ? (s-'a'+0xa) :(((s>= 'A' && s<='F') ? (s-'A'+0xa) :0xffff))));
}

int str_to_hex(strHexDest_t dest, uint8_t *p, int size)
{
	int i;
	int32_t high_4bits;
	int32_t low_4bits;
	for( i = 0 ; i < (size / 2) ; i++)
	{
		high_4bits = char2hex(p[2*i]);
		low_4bits = char2hex(p[2*i+1]);
		if (high_4bits == 0xffff || low_4bits == 0xffff) {
			return -1;
		}
		dest[i] = high_4bits << 4 | low_4bits;
	}
	return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
