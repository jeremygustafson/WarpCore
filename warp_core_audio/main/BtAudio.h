/*
 * BtAudio.h
 *
 *  Created on: 25 Feb 2018
 *      Author: dancl
 */

#ifndef MAIN_BTAUDIO_H_
#define MAIN_BTAUDIO_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define BT_DISCOVERY_DELAY_MS	(5000)

#define BIT_BT_AUDIO_READY		(1 << 0)
#define BIT_BT_PLAYING			(1 << 1)
#define BIT_BT_STOPPED			(1 << 2)

/* event for handler "bt_av_hdl_stack_up */
enum {
    BT_APP_EVT_STACK_UP = 0,
};

/* A2DP global state */
enum {
    APP_AV_STATE_IDLE,
	APP_AV_STATE_PENDING_DISCOVERING,
    APP_AV_STATE_DISCOVERING,
    APP_AV_STATE_DISCOVERED,
    APP_AV_STATE_UNCONNECTED,
    APP_AV_STATE_CONNECTING,
    APP_AV_STATE_CONNECTED,
    APP_AV_STATE_DISCONNECTING,
};

/* sub states of APP_AV_STATE_CONNECTED */
enum {
    APP_AV_MEDIA_STATE_IDLE,
    APP_AV_MEDIA_STATE_STARTING,
    APP_AV_MEDIA_STATE_STARTED,
    APP_AV_MEDIA_STATE_STOPPING,
};

#ifdef __cplusplus
  extern "C" {
#endif

extern EventGroupHandle_t btEventGroup;

void initBtAudio(const char *deviceName);
void playBtAudioFile(FILE *file, fpos_t dataStart, uint32_t repeatCount, uint32_t repeatDelay);

#ifdef __cplusplus
  }
#endif

#endif /* MAIN_BTAUDIO_H_ */
