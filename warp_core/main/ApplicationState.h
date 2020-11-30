/*
 * ApplicationState.h
 *
 *  Created on: 27 Feb 2018
 *      Author: dancl
 */

#ifndef MAIN_APPLICATIONSTATE_H_
#define MAIN_APPLICATIONSTATE_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define PATTERN_IDLE			0
#define PATTERN_WARP			1
#define PATTERN_RED_ALERT		2
#define PATTERN_RED_ALERT_VGR	3
#define PATTERN_BREACH			4
#define PATTERN_RAINBOW			5
#define PATTERN_RAINBOW_ROWS	6
#define PATTERN_RAINBOW_COLS	7
#define PATTERN_RAINBOW_BOTH	8
#define PATTERN_STACK			9
#define PATTERN_SPIRAL			10
#define PATTERN_SOLID_COLOUR	11
#define PATTERN_KNIGHT_RIDER	12
#define PATTERN_OFF				13

typedef struct colour
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} colour_t;

typedef struct appStateChangeReq
{
	char *jsonStr;
} appStateChange_t;

extern volatile uint8_t displayPattern;
extern volatile bool soundOn;
extern volatile bool customColourActive;
extern volatile colour_t requestedColour;

extern xQueueHandle appStateQueueHandle;
extern xQueueHandle animChangedQueueHandle;

void initAppState();

#ifdef __cplusplus
  }
#endif

#endif /* MAIN_APPLICATIONSTATE_H_ */
