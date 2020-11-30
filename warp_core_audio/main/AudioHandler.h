/*
 * AudioHandler.h
 *
 *  Created on: 3 Mar 2018
 *      Author: dancl
 */

#ifndef MAIN_AUDIOHANDLER_H_
#define MAIN_AUDIOHANDLER_H_

#include <stdint.h>
#include "WavPlayer.h"

#define WAV_CORE			(1)
#define WAV_CORE_MAX		(2)
#define WAV_RED_ALERT		(3)
#define WAV_RED_ALERT_VGR	(4)
#define WAV_BREACH			(5)

void executeCommand(uint8_t command, WavPlayer *player);

#endif /* MAIN_AUDIOHANDLER_H_ */
