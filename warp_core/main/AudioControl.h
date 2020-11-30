/*
 * AudioControl.h
 *
 *  Created on: 3 Mar 2018
 *      Author: dancl
 */

#ifndef MAIN_AUDIOCONTROL_H_
#define MAIN_AUDIOCONTROL_H_

#define WAV_CORE			(1)
#define WAV_CORE_MAX		(2)
#define WAV_RED_ALERT		(3)
#define WAV_RED_ALERT_VGR	(4)
#define WAV_BREACH			(5)

#include <stdint.h>

#ifdef __cplusplus
  extern "C" {
#endif

void initAudioControl();
void playAudioFile(uint8_t file);

#ifdef __cplusplus
  }
#endif

#endif /* MAIN_AUDIOCONTROL_H_ */
