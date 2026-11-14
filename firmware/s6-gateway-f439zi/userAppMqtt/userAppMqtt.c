/**
 * @file userAppMqtt.c
 *
 * @date Feb 13, 2025
 * @author OULMAKI
 */

#include "debugTerminal.h"
#include "FreeRTOS.h"
#include "main.h"
#include "cmsis_os.h"
#include "lwip.h"
#include <ip_addr.h>
#include <mqtt.h>
#include "semphr.h"
#include "userAppNrf.h"
#include "userAppMqtt.h"

#define IP_ADDRESS_BROKER "192.0.2.1"
SemaphoreHandle_t publishSemaphore;

#define JOINING "GEII/SAE6/joining/011"
#define REQUEST "GEII/SAE6/request/011"

static void mqtt_pub_request_cb(void *arg, err_t result);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_sub_request_cb(void *arg, err_t result);
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);

/*
void mqtt_Init(void*param){

     mqtt_client_t *client = mqtt_client_new();
     err_t error_connect;

     publishSemaphore = xSemaphoreCreateBinary();

     if(!publishSemaphore){
         debugLogInfoTrace("Semaphore pas cree\n");
     }

     if (client != NULL) {

         do{
             error_connect = connect(client);
         }

         while(error_connect);

         debugLogInfoTrace("Connextion client OK\n");

         if(error_connect != ERR_OK){
             debugLogInfoTrace("Connextion client impossible - Error = %d\n",error_connect);
         }

     }

     else{
         debugLogInfoTrace("Client pas cree\n");
     }

     xSemaphoreTake(publishSemaphore,portMAX_DELAY);

     publish_TX(client, 0);

     while(1){

        vTaskDelay(1000);

        if(publish(client,0) != ERR_OK){
            debugLogInfoTrace("Error publish\n");
        }

    }

}

*/
void mqtt_Init(void *param) {

    mqtt_client_t *client = mqtt_client_new();
    err_t error_connect;

    uint8_t receivedData[24];

    publishSemaphore = xSemaphoreCreateBinary();

    if (!publishSemaphore) {
        debugLogInfoTrace("Semaphore pas cree\n");
    }

    if (client != NULL) {

        do {

            error_connect = connect(client);

        }

        while (error_connect);

        debugLogInfoTrace("Connexion client OK\n");

        if (error_connect != ERR_OK) {

            debugLogInfoTrace("Connexion client impossible - Error = %d\n", error_connect);

        }

    }

    else {

        debugLogInfoTrace("Client pas créé\n");

    }

    xSemaphoreTake(publishSemaphore, portMAX_DELAY);

    while (1) {

       mqtt_queue_recive(receivedData);

       publish_with_payload(client, receivedData, sizeof(receivedData));


    }

    vTaskDelay(1000);

}




err_t connect(mqtt_client_t *client){

    struct mqtt_connect_client_info_t ci;
    err_t err;

    ip_addr_t IP;

    ip4addr_aton(IP_ADDRESS_BROKER, &IP);

    memset(&ci, 0, sizeof(ci));
    ci.client_id = "client_011";

    err = mqtt_client_connect(client, &IP, MQTT_PORT, mqtt_connection_cb, 0, &ci);

    return err;

}

err_t publish(mqtt_client_t *client, void *arg)
{
  const char *pub_payload= "011_test";

  err_t err;
  u8_t qos = 0;
  u8_t retain = 0;

  err = mqtt_publish(client, "GEII/SAE6/Test/011", pub_payload, strlen(pub_payload), qos, retain, mqtt_pub_request_cb, arg);

  return err;

}

err_t publish_TX(mqtt_client_t *client, void *arg)
{
  const char *pub_payload= "TX_test_011";

  err_t err;
  u8_t qos = 0;
  u8_t retain = 0;

  err = mqtt_publish(client,REQUEST, pub_payload, strlen(pub_payload), qos, retain, mqtt_pub_request_cb, arg);

  return err;

}



err_t publish_with_payload(mqtt_client_t *client, const uint8_t *data, uint16_t len)
{
    err_t err;
    u8_t qos = 0;
    u8_t retain = 0;

    err = mqtt_publish(client, "GEII/SAE6/Test/011", (const char *)data, len, qos, retain, mqtt_pub_request_cb, NULL);

    if (err != ERR_OK) {
        debugLogInfoTrace("Erreur publication avec payload : %d\n", err);
    }

    return err;
}


static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    debugLogInfoTrace("Incoming publish at topic %s with total length %u\n", topic, (unsigned int)tot_len);

}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    debugLogInfoTrace("Incoming publish payload with length %d, flags %u\n", len, (unsigned int)flags);

    char incoming_data[len + 1];

    memcpy(incoming_data, data, len);

    incoming_data[len] = '\0';

    debugLogInfoTrace("Data = %s\n", incoming_data);

}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status){

    err_t err;

     if(status == MQTT_CONNECT_ACCEPTED) {

          debugLogInfoTrace("cb - client connecte\n");

          xSemaphoreGive(publishSemaphore);

          mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);

          err = mqtt_subscribe(client, JOINING, 0, mqtt_sub_request_cb, arg);

          if(err != ERR_OK){
              debugLogInfoTrace("mqtt_subscribe Error : %d \n",err);
          }
          else{
              debugLogInfoTrace("mqtt_subscribe joining OK");
          }
     }

}

static void mqtt_pub_request_cb(void *arg, err_t result){

  if(result != ERR_OK) {
      debugLogInfoTrace("Error = %d\n",ERR_OK);
  }

}


static void mqtt_sub_request_cb(void *arg, err_t result)
{
    debugLogInfoTrace("Subscribe result: %d\n", result);
}


int32_t tasks_Init (void){

    if(xTaskCreate(mqtt_Init, "mqtt_Init", 512, 0, osPriorityNormal, 0) == pdFALSE){
        return -1;
    }

    return 0;

}
