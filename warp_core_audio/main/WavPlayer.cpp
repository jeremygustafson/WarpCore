/*
 * WavPlayer.cpp
 *
 *  Created on: 23 Feb 2018
 *      Author: dancl
 *
 *  Wav playback code from: https://github.com/nhatuan84/esp32-i2s-sdcard-wav-player
 */

#include "WavPlayer.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_event_loop.h"
#include "BtAudio.h"

extern "C" {
	#include "esp_vfs.h"
	#include "esp_vfs_fat.h"
}

const char *WAV_TAG = "Wav";

#define CCCC(c1, c2, c3, c4) ((c4 << 24) | (c3 << 16) | (c2 << 8) | c1)
#define PLAY_BIT	(1 << 0)
#define STOP_BIT	(1 << 1)
#define PLAYING_BIT	(1 << 2)
#define READY_BIT	(1 << 3)

WavPlayer::WavPlayer(uint8_t bckPin, uint8_t lrckPin, uint8_t dataPin, i2s_port_t portNum)
	: TaskClass("audio_dispatch", 10, 2048),
	portNum(portNum)
{
	i2sConfig.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
	i2sConfig.sample_rate = 44100;
	i2sConfig.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
	i2sConfig.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
	i2sConfig.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB);
	i2sConfig.dma_buf_count = 4;
	i2sConfig.dma_buf_len = 440;
	i2sConfig.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
	//i2sConfig.use_apll = true;

	pinConfig.bck_io_num = bckPin;
	pinConfig.ws_io_num = lrckPin;
	pinConfig.data_out_num = dataPin;
	pinConfig.data_in_num = -1;	// Not used

	eventGroup = xEventGroupCreate();
	assert(eventGroup != NULL);
}

WavPlayer::~WavPlayer()
{
	close();
	vEventGroupDelete(eventGroup);
}

void WavPlayer::task()
{
	while (true)
	{
		// Wait for a play request to come in
		ESP_LOGI(WAV_TAG, "Waiting for playback command");
		xEventGroupSetBits(eventGroup, READY_BIT);
		EventBits_t bits = xEventGroupWaitBits(eventGroup, PLAY_BIT, true, true, portMAX_DELAY);
		if (bits == 0) // Timeout hit
			continue;

		// Play!
		xEventGroupClearBits(eventGroup, READY_BIT);
		xEventGroupSetBits(eventGroup, PLAYING_BIT);

		EventBits_t btBits = xEventGroupGetBits(btEventGroup);
		if ((btBits & BIT_BT_AUDIO_READY) == BIT_BT_AUDIO_READY)
			playBt();
		else
			playI2s();

		fclose(wavFile);
		wavFile = NULL;
		ESP_LOGI(WAV_TAG, "Playback complete");
		xEventGroupClearBits(eventGroup, PLAYING_BIT | STOP_BIT);
	}
}

void WavPlayer::init()
{
	if (driverInstalled)
		return;

	ESP_LOGI(WAV_TAG, "Initialising I2S driver");
	ESP_ERROR_CHECK(i2s_driver_install(portNum, &i2sConfig, 0, NULL));
	ESP_ERROR_CHECK(i2s_set_pin(portNum, &pinConfig));

	driverInstalled = true;
}

void WavPlayer::close()
{
	stopPlayback();

	if (!driverInstalled)
		return;

	ESP_ERROR_CHECK(i2s_driver_uninstall(portNum));
	driverInstalled = false;
}

bool WavPlayer::playAudioFile(FILE *file, uint32_t _repeatCount, uint32_t _repeatDelay)
{
	if (!driverInstalled)
		return false;
	if (file == NULL)
	{
		ESP_LOGE(WAV_TAG, "File not provided");
		return false;
	}

	stopPlayback();
	xEventGroupWaitBits(eventGroup, READY_BIT, false, true, portMAX_DELAY);

	// Initial state
	repeatCount = _repeatCount;
	repeatDelay = _repeatDelay;
	wavFile = file;

	// Read in actual data
	if (!readHeader())
		return false;

	i2s_channel_t channels;
	switch (wavProps.numChannels)
	{
	case 1:
		channels = I2S_CHANNEL_MONO;
		break;

	case 2:
		channels = I2S_CHANNEL_STEREO;
		break;

	default:
		ESP_LOGE(WAV_TAG, "Unsupported channel count");
		return false;
	}

	if (wavProps.audioFormat != 1)
	{
		ESP_LOGE(WAV_TAG, "Audio must be uncompressed PCM");
		return false;
	}

	ESP_ERROR_CHECK(i2s_set_clk(portNum, wavProps.sampleRate, (i2s_bits_per_sample_t)wavProps.bitsPerSample, channels));

	// Tell worker to play
	xEventGroupClearBits(eventGroup, STOP_BIT);
	xEventGroupSetBits(eventGroup, PLAY_BIT);

	return true;
}

void WavPlayer::stopPlayback()
{
	// Tell worker to stop
	xEventGroupSetBits(eventGroup, STOP_BIT);
}

bool WavPlayer::readHeader()
{
	const size_t riffLen = sizeof(wavRiff_t);
	const size_t wavPropsLen = sizeof(wavProperties_t);

	size_t read = fread(&wavRiff, 1, riffLen, wavFile);
	if (read < riffLen)
	{
		ESP_LOGE(WAV_TAG, "File too small (riffLen)");
		return false;
	}

	// RIFF header
	if (wavRiff.chunkID != CCCC('R', 'I', 'F', 'F'))
	{
		ESP_LOGE(WAV_TAG, "Header invalid (wavRiff.chunkID)");
		return false;
	}
	if (wavRiff.format != CCCC('W', 'A', 'V', 'E'))
	{
		ESP_LOGE(WAV_TAG, "Header invalid (wavRiff.format)");
		return false;
	}

	// Wav properties header
	read = fread((void*)&wavProps, 1, wavPropsLen, wavFile);
	if (read < wavPropsLen)
	{
		ESP_LOGE(WAV_TAG, "File too small (wavPropsLen)");
		return false;
	}

	// Header data
	uint32_t chunkId, chunkSize;
	if (!read4bytes(&chunkId))
	{
		ESP_LOGE(WAV_TAG, "File too small (chunkId)");
		return false;
	}
	if (chunkId != CCCC('d', 'a', 't', 'a'))
	{
		ESP_LOGE(WAV_TAG, "Header data invalid");
		return false;
	}

	if (!read4bytes(&chunkSize))
	{
		ESP_LOGE(WAV_TAG, "File too small (chunkSize)");
		return false;
	}

	return true;
}

bool WavPlayer::read4bytes(uint32_t *destination)
{
	size_t read = fread(destination, 1, 4, wavFile);
	if (read < 4)
		return false;

	return true;
}

uint32_t WavPlayer::readBytes(uint8_t *destination, size_t len)
{
	size_t read = fread(destination, 1, len, wavFile);
	return read;
}

void WavPlayer::playI2s()
{
	ESP_LOGI(WAV_TAG, "Playing to I2S");

	bool i2sStarted = false;
	fpos_t initialPos;
	if (fgetpos(wavFile, &initialPos) != 0)
		return;

	for (uint32_t currentRepeat = 0; currentRepeat <= repeatCount; ++currentRepeat)
	{
		ESP_LOGI(WAV_TAG, "Beginning playback");
		if (fsetpos (wavFile, &initialPos) != 0)
			break;
		if (!i2sStarted)
		{
			i2s_zero_dma_buffer(portNum); // Perform an initial clear to remove audio oddities
			ESP_ERROR_CHECK(i2s_start(portNum));
			i2sStarted = true;
		}

		bool stopRequested = false;
		while (true)
		{
			size_t len = fread(sampleBuffer, 1, sizeof(sampleBuffer), wavFile);
			if (len <= 0)
				break;

			// Stop requested
			if ((xEventGroupGetBits(eventGroup) & STOP_BIT) > 0)
			{
				stopRequested = true;
				break;
			}

			// Get next buffer piece and send down the pipe
			int processedBytes = i2s_write_bytes(portNum, (char *)sampleBuffer, len, portMAX_DELAY);
			if (processedBytes < len)
			{
				uint32_t delta = len - processedBytes;
				fseek(wavFile, -delta, SEEK_CUR);
			}
			taskYIELD();
		}

		if (stopRequested)
			break;

		if (repeatDelay > 0)
		{
			ESP_ERROR_CHECK(i2s_stop(portNum));
			i2sStarted = false;
			vTaskDelay(repeatDelay / portTICK_PERIOD_MS);
		}
	}

	if (i2sStarted)
	{
		i2s_zero_dma_buffer(portNum);
		ESP_ERROR_CHECK(i2s_stop(portNum));
	}
}

void WavPlayer::playBt()
{
	ESP_LOGI(WAV_TAG, "Playing to BT");

	// Currently at the data position, save it
	fpos_t initialPos;
	if (fgetpos(wavFile, &initialPos) != 0)
		return;

	// Send request down to BT
	playBtAudioFile(wavFile, initialPos, repeatCount, repeatDelay);

	// Wait for BT to start playing
	EventBits_t btBits = xEventGroupWaitBits(btEventGroup, BIT_BT_PLAYING, false, true, 1000 / portTICK_PERIOD_MS);
	if ((btBits & BIT_BT_PLAYING) == 0)
		return;

	// Wait for BT to stop playing
	xEventGroupWaitBits(btEventGroup, BIT_BT_STOPPED, false, true, portMAX_DELAY);
}
