/*
 * WiFi.h
 *
 *  Created on: 27 Feb 2018
 *      Author: dancl
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define WIFI_CONNECTED_BIT (1 << 0)

#ifdef __cplusplus
  extern "C" {
#endif

extern EventGroupHandle_t wifi_event_group;

void initWifi();

#ifdef __cplusplus
  }
#endif

#endif /* MAIN_WIFI_H_ */
