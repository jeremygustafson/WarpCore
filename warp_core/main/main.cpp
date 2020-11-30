/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "Arduino.h"

#include "WiFi.h"
#include "ApplicationState.h"
#include "WebServer.h"
#include "WarpCoreVisual.h"
#include "WarpCoreAnimator.h"
#include "StandardCoreAnimation.h"
#include "RedAlertAnimation.h"
#include "RedAlertVoyagerAnimation.h"
#include "RainbowAnimation.h"
#include "StackAnimation.h"
#include "SpiralAnimation.h"
#include "CoreBreachAnimation.h"
#include "SolidColourAnimation.h"
#include "KnightRiderAnimation.h"
#include "OffAnimation.h"
#include "AudioControl.h"

#include "Config.h"

extern "C" {

#include "esp_spiffs.h"

WarpCoreVisual warpCore(MAX_CHANNEL_BRIGHTNESS, MAX_ALL_BRIGHTNESS);
WarpCoreAnimator coreAnimator;
StandardCoreAnimation standardAnim(warpCore);
RedAlertAnimation redAlertAnim(warpCore);
RedAlertVoyagerAnimation redAlertVgrAnim(warpCore);
RainbowAnimation rainbowAnim(warpCore);
StackAnimation stackAnim(warpCore);
SpiralAnimation spiralAnim(warpCore);
CoreBreachAnimation coreBreachAnim(warpCore);
SolidColourAnimation solidAnim(warpCore);
KnightRiderAnimation riderAnim(warpCore);
OffAnimation offAnim(warpCore);

coreSection_t coreSections[NUM_SECTIONS];

const char *MAIN_TAG = "MainLoop";

void initSpiffs();
void initLeds();
void setAnim(uint8_t pattern);

void app_main()
{
    ESP_LOGI(MAIN_TAG, "Init");

    // Initialize NVS.
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	ESP_LOGI(MAIN_TAG, "Init SPIFFS");
	initSpiffs();

	ESP_LOGI(MAIN_TAG, "Init WiFi");
	initWifi();

	ESP_LOGI(MAIN_TAG, "Init Arduino Component");
	initArduino();

	ESP_LOGI(MAIN_TAG, "Init App State");
	initAppState();

	ESP_LOGI(MAIN_TAG, "Init audio UART");
	initAudioControl();

	ESP_LOGI(MAIN_TAG, "Init LEDs");
	initLeds();

	// Initial animation
	ESP_LOGI(MAIN_TAG, "Set initial animation");
	setAnim(displayPattern);
	//coreAnimator.setAnimation(&riderAnim);

	// Web server
	ESP_LOGI(MAIN_TAG, "Init Web Server");
	initWebServer();

	ESP_LOGI(MAIN_TAG, "Init complete");

    while (true)
	{
    	uint8_t newPattern;
    	if (pdTRUE != xQueueReceive(animChangedQueueHandle, &newPattern, (portTickType)portMAX_DELAY))
    		continue;

    	setAnim(newPattern);
    	//vTaskDelay(100000 / portTICK_PERIOD_MS);
	}
}

void initSpiffs()
{
	// Init SPIFFs functionality
	/*esp_vfs_spiffs_conf_t conf = {
		  .base_path = "/spiffs",
		  .partition_label = NULL,
		  .max_files = 5,
		  .format_if_mount_failed = false
	};
	esp_err_t ret = esp_vfs_spiffs_register(&conf);
	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
			ESP_LOGE(MAIN_TAG, "Failed to mount or format filesystem");
		else if (ret == ESP_ERR_NOT_FOUND)
			ESP_LOGE(MAIN_TAG, "Failed to find SPIFFS partition");
		else
			ESP_LOGE(MAIN_TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));

		ESP_LOGE(MAIN_TAG, "SPIFFS Error");
		return;
	}

	// Perform basic diagnostic of partition
	size_t total = 0, used = 0;
	ret = esp_spiffs_info(NULL, &total, &used);
	if (ret != ESP_OK)
		ESP_LOGE(MAIN_TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
	else
		ESP_LOGI(MAIN_TAG, "Partition size: total: %d, used: %d", total, used);

	// Perform diagnostic of image
	struct stat st;
	if (stat("/spiffs/web/index.htm", &st) < 0)
		ESP_LOGE(MAIN_TAG, "Could not get stat on index.htm");
	else
		ESP_LOGI(MAIN_TAG, "Index size: %d", (int)st.st_size);

	ESP_LOGI(MAIN_TAG, "SPIFFS Complete");*/
}

void initLeds()
{
	memset(coreSections, 0, sizeof(coreSections));
	coreSections[0].numLeds = LED_COUNT_ROW * LED_ROWS_PER_SECTION;
	coreSections[0].numRows = LED_ROWS_PER_SECTION;
	coreSections[0].pin = PIN_ROW_1;
	coreSections[0].winding = REVERSE;

	// Fake row to cater for bar across segments
	coreSections[1].numLeds = LED_COUNT_ROW * (LED_ROWS_PER_SECTION);
	coreSections[1].numRows = LED_ROWS_PER_SECTION;
	coreSections[1].pin = PIN_ROW_2;
	coreSections[1].winding = REVERSE;

	coreSections[2].numLeds = LED_COUNT_CHAMBER;
	coreSections[2].numRows = 1;
	coreSections[2].pin = PIN_ROW_3;
	coreSections[2].rowType = CENTRAL_CHAMBER;
	//coreSections[2].winding = REVERSE;

	// Fake row to cater for bar across segments
	coreSections[3].numLeds = LED_COUNT_ROW * (LED_ROWS_PER_SECTION);
	coreSections[3].numRows = LED_ROWS_PER_SECTION;
	coreSections[3].pin = PIN_ROW_4;

	coreSections[4].numLeds = LED_COUNT_ROW * LED_ROWS_PER_SECTION;
	coreSections[4].numRows = LED_ROWS_PER_SECTION;
	coreSections[4].pin = PIN_ROW_5;

	warpCore.init(coreSections, NUM_SECTIONS);
}

void setAnim(uint8_t pattern)
{
	switch (pattern)
	{
	case PATTERN_IDLE:
		standardAnim.setType(CORE_ANIM_IDLE);
		coreAnimator.setAnimation(&standardAnim);
		break;

	case PATTERN_WARP:
		standardAnim.setType(CORE_ANIM_WARP);
		coreAnimator.setAnimation(&standardAnim);
		break;

	case PATTERN_RED_ALERT:
		coreAnimator.setAnimation(&redAlertAnim);
		break;

	case PATTERN_RED_ALERT_VGR:
		coreAnimator.setAnimation(&redAlertVgrAnim);
		break;

	case PATTERN_BREACH:
		coreAnimator.setAnimation(&coreBreachAnim);
		break;

	case PATTERN_RAINBOW:
		rainbowAnim.setType(RAINBOW_FULL);
		coreAnimator.setAnimation(&rainbowAnim);
		break;

	case PATTERN_RAINBOW_ROWS:
		rainbowAnim.setType(RAINBOW_CHASE_ROW);
		coreAnimator.setAnimation(&rainbowAnim);
		break;

	case PATTERN_RAINBOW_COLS:
		rainbowAnim.setType(RAINBOW_CHASE_COL);
		coreAnimator.setAnimation(&rainbowAnim);
		break;

	case PATTERN_RAINBOW_BOTH:
		rainbowAnim.setType(RAINBOW_CHASE_BOTH);
		coreAnimator.setAnimation(&rainbowAnim);
		break;

	case PATTERN_STACK:
		coreAnimator.setAnimation(&stackAnim);
		break;

	case PATTERN_SPIRAL:
		coreAnimator.setAnimation(&spiralAnim);
		break;

	case PATTERN_SOLID_COLOUR:
		coreAnimator.setAnimation(&solidAnim);
		break;

	case PATTERN_KNIGHT_RIDER:
		coreAnimator.setAnimation(&riderAnim);
		break;

	case PATTERN_OFF:
		coreAnimator.setAnimation(&offAnim);
		break;

	default:
		ESP_LOGW(MAIN_TAG, "Unsupported animation: %i", pattern);
		break;
	}
}

}
