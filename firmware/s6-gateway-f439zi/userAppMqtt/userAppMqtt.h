/**
 * @file userAppMqtt.h
 *
 * @date Feb 13, 2025
 * @author OULMAKI
 */

#include <mqtt.h>

#ifndef USERAPPMQTT_H_
#define USERAPPMQTT_H_

int32_t tasks_Init (void);
err_t connect(mqtt_client_t *client);
err_t publish(mqtt_client_t *client, void *arg);
err_t publish_TX(mqtt_client_t *client, void *arg);
err_t publish_with_payload(mqtt_client_t *client, const uint8_t *data, uint16_t len);

#endif /* USERAPPMQTT_H_ */
