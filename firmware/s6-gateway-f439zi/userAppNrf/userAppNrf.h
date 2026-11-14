/**
 * @file userAppNrf.h
 *
 * @date Mar 4, 2025
 * @author OULMAKI
 */

#ifndef USERAPPNRF_H_
#define USERAPPNRF_H_

#include <stdint.h>

void userAppNrf(void);
void userAppNrfInit(void);
void handle_receive_data(void*parm);
uint8_t mqtt_queue_recive(uint8_t data);
int32_t task_nrf_Init (void);

#endif /* USERAPPNRF_H_ */
