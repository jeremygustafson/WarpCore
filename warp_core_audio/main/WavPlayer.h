/*
 * WavPlayer.h
 *
 *  Created on: 23 Feb 2018
 *      Author: dancl
 */

#ifndef MAIN_WAVPLAYER_H_
#define MAIN_WAVPLAYER_H_

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/i2s.h"
#include "TaskCPP.h"


typedef struct wavRiff_s {
    uint32_t chunkID;
    uint32_t chunkSize;
    uint32_t format;
} wavRiff_t;

typedef struct wavProperties_s {
    uint32_t chunkID;
    uint32_t chunkSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
} wavProperties_t;

class WavPlayer : public TaskClass
{
public:
	WavPlayer(uint8_t bckPin, uint8_t lrckPin, uint8_t dataPin, i2s_port_t portNum);
	virtual ~WavPlayer();

	void init();
	void close();
	bool playAudioFile(FILE *file, uint32_t repeatCount = 0, uint32_t repeatDelay = 0);
	void stopPlayback();

protected:
	virtual void task();

private:
	// Hardware
	i2s_port_t portNum;
	i2s_config_t i2sConfig = {};
	i2s_pin_config_t pinConfig = {};
	bool driverInstalled = false;

	// File state
	FILE *wavFile = NULL;
	wavRiff_t wavRiff;
	wavProperties_t wavProps;

	// Sync
	EventGroupHandle_t eventGroup;

	// Buffers & playback requirements
	uint8_t sampleBuffer[440];
	uint32_t repeatCount = 0;
	uint32_t repeatDelay = 0;

	// Read wave data
	bool readHeader();
	bool read4bytes(uint32_t *destination);
	uint32_t readBytes(uint8_t *destination, size_t len);

	// Playback
	void playI2s();
	void playBt();
};

#endif /* MAIN_WAVPLAYER_H_ */
