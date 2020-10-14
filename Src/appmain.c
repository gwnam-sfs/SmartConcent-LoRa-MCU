/*
 * appmain.c
 *
 *  Created on: 2020. 7. 10.
 *      Author:
 */


#include "main.h"
#include "usart.h"
#include "gpio.h"

#include <stdio.h>
#include <string.h>
#include "UartRingbuffer.h"
#include "LoRa\lora_atcmd.h"

int dataSize;
int dataSize_c;
int testCount = 0;
//int count = 0;

char dataFromLora[150] = {0,}; 	// 150byte char buffer store message from lora module
char dataFromConsole[150] = {0,};// 150byte char buffer store message from console(PC)

//void system_init(void)
//{
//	Ringbuf_init();
//	SysTick_Config(SystemCoreClock / 1000);
//}

int app_main(void)
{
//	system_init();
	printf("main program start...\r\n");
	printf("reset..\r\n");

	lora_Init();

	while(1)
	{

		if (isgetc_from_console()) // Console에서 도착한 문자가 있으면
		{
			dataSize_c = get_console_message(dataFromConsole);
#ifdef OTB
			lora_OTB_cmd_test(dataFromConsole, dataSize_c);
#endif
			console_message_analysis (dataFromConsole, dataSize_c);

		}//if (isgetc_from_console())



		if (isgetc_from_lora())
		{
			dataSize = get_lora_message (dataFromLora);
			if (dataSize == -1) {
				printf ("AT_RX_ERROR_GW\r\n");
				return -1;
			}
			printf ("%s", dataFromLora);
		}

		lora_message_analysis(dataFromLora, dataSize);

		if (dataFromLora[0] != 0x00) {
			if (dataFromLora[strlen(dataFromLora) - 1] != '\n') {
				DEBUG_PRINTF("[GWD] %d, %s\r\n", strlen(dataFromLora), dataFromLora);
			}
			else {
				DEBUG_PRINTF("[GWD] %d, %s", strlen(dataFromLora), dataFromLora);
			}

			/* Callback function this position -----------------------------------*/
			mgmtCmd_analysis(dataFromLora, strlen(dataFromLora));

			/*--------------------------------------------------------------------*/
			memset(dataFromLora, 0x00, dataSize);
		}

		lora_command_process();
#ifdef OTB
		if (flag.loraCycleUplink == 1 && flag.loraTransmit == 1) lora_OTB_CyUp_test();
#endif
	}//while(1)
	return 0;
}
