/*
 * AudioHandler.c
 *
 *  Created on: 3 Mar 2018
 *      Author: dancl
 */

#include "AudioHandler.h"
#include "esp_log.h"
#include <errno.h>
#include "FS.h"
#include "SPIFFS.h"

extern "C" {
	#include "esp_vfs.h"
	#include "esp_vfs_fat.h"
}

void executeCommand(uint8_t command, WavPlayer *player)
{
	FILE *wavFile = NULL;
	ESP_LOGD("Audio", "Audio file: %i", command);

	switch (command)
	{
	case WAV_CORE:
		wavFile = fopen("/spiffs/wav/core.wav", "rb");
		break;

	case WAV_CORE_MAX:
		wavFile = fopen("/spiffs/wav/core_max.wav", "rb");
		break;

	case WAV_RED_ALERT:
		wavFile = fopen("/spiffs/wav/redalert.wav", "rb");
		break;

	case WAV_RED_ALERT_VGR:
		wavFile = fopen("/spiffs/wav/redalert_vgr.wav", "rb");
		break;

	case WAV_BREACH:
		wavFile = fopen("/spiffs/wav/breach.wav", "rb");
		break;

	default:
		ESP_LOGW("Audio", "No audio file match for %i", command);
		return;
	}
	
	if (wavFile != NULL)
		player->playAudioFile(wavFile, 0, 0);
	else
		ESP_LOGE("Audio", "wavFile is NULL. Error code: %d : %s\n", errno, strerror(errno));
}
