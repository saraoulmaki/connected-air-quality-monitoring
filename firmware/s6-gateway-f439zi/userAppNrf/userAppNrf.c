/**
 * @file userAppNrf.c
 *
 * @date Mar 4, 2025
 * @author OULMAKI
 */

#include "FreeRTOS.h"
#include "userAppNrf.h"
#include "debugTerminal.h"
#include "nrf24l01_stm.h"
#include "spi.h"
#include "userAppMqtt.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "cmsis_os.h"


#define USER_APP_SEND_PERIOD 1000
#define USER_RECEPTION  11

#define IP_ADDRESS_BROKER "192.0.2.1"

SemaphoreHandle_t nrfSemaphore;
QueueHandle_t nrfDataQueue;

	static const uint8_t rx_address[5] = {1, 2, 3, 4, USER_RECEPTION};
	static const uint8_t tx_address[5] = {1, 2, 3, 4, 5};

	nrf24l01 nrf;
	uint8_t nrf_data[24];

typedef enum
{
	USER_APP_TASK_STATE_INIT = 0, USER_APP_TASK_STATE_IDLE,
} UserAppTaskState_t;



void userAppNrfInit(void)
{
	nrfSemaphore = xSemaphoreCreateBinary();
	nrfDataQueue = xQueueCreate(24,sizeof(nrf_data));

	    {
	        nrf24l01_config config;
	        config.data_rate        = NRF_DATA_RATE_250KBPS;
	        config.tx_power         = NRF_TX_PWR_0dBm;
	        config.crc_width        = NRF_CRC_WIDTH_2B;
	        config.addr_width       = NRF_ADDR_WIDTH_5;
	        config.payload_length   = sizeof(nrf_data);    // maximum is 32 bytes
	        config.retransmit_count = 15;   // maximum is 15 times
	        config.retransmit_delay = 0x0F; // 4000us, LSB:250us
	        config.rf_channel       = 0;
	        config.rx_address       = rx_address;
	        config.tx_address       = tx_address;
	        config.rx_buffer        = nrf_data;

	        config.spi         = &hspi4;
	        config.spi_timeout = 10; // milliseconds
	        config.ce_port     = IO_MIC1_CE_GPIO_Port;
	        config.ce_pin      =IO_MIC1_CE_Pin;
	        config.irq_port    = IO_MIC1_INT_GPIO_Port;
	        config.irq_pin     = IO_MIC1_INT_Pin;
	        config.csn_port    = IO_MIC1_CSN_GPIO_Port;
	        config.csn_pin     = IO_MIC1_CSN_Pin;

	        nrf_init(&nrf, &config);

	        if(nrf_init(&nrf, &config) != NRF_OK){
	            debugLogErrorTrace("NRF Init Fail");
	        }
	        else{
	            debugLogInfoTrace("NRF Init OK");
	        }

	        xTaskCreate(handle_receive_data, "handle_receive_data", 512, 0, osPriorityNormal, 0);
	        tasks_Init();
}

}

void handle_receive_data(void*parm){

	int i;

	while(1){

		debugLogInfoTrace("En attente de Semaphore\n");

		if(xSemaphoreTake(nrfSemaphore, portMAX_DELAY) == pdTRUE){

			debugLogInfoTrace("\nSemaphore pris\n");


			for (i = 0; i < 24; i++) {
				debugLogInfoTrace("%02X ", nrf_data[i]);
			}

			debugLogInfoTrace("\n");


			if (xQueueSend(nrfDataQueue, &nrf_data, portMAX_DELAY) != pdPASS) {
				debugLogErrorTrace("Erreur envoi queue\n");
			}

		}

	}

}

uint8_t mqtt_queue_recive(uint8_t data){

	  if (!nrfDataQueue){
		  return -1;
	  }

	  if (xQueueReceive(nrfDataQueue, &data, portMAX_DELAY) == pdPASS){
		  return -1;
	  }

	  return 0;

}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	BaseType_t context;

	if(GPIO_Pin != IO_MIC1_INT_Pin) {
		return;
	}

	xSemaphoreGiveFromISR(nrfSemaphore,pdFALSE);

	nrf_irq_handler(&nrf);

	portYIELD_FROM_ISR(context);

}
