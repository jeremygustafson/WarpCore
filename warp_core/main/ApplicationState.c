/*
 * ApplicationState.c
 *
 *  Created on: 27 Feb 2018
 *      Author: dancl
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "cJSON.h"

#include "ApplicationState.h"
#include "WiFi.h"

const char *APP_TAG = "AppState";

volatile uint8_t displayPattern = PATTERN_IDLE;
volatile bool soundOn = true;
volatile bool customColourActive = false;
volatile colour_t requestedColour = {};
xQueueHandle appStateQueueHandle = NULL;
xQueueHandle animChangedQueueHandle = NULL;

// Config storage
nvs_handle configHandle;
const char *KEY_DISPLAY_PATTERN = "displayPattern";
const char *KEY_SOUND_ON = "soundOn";
const char *KEY_CUSTOM_COLOUR_ACTIVE = "colourActive";
const char *KEY_CUSTOM_COLOUR = "cColour";

void appStateTask(void *pvParameter);
void parseStateJson(const char *jsonStr);

bool parseSound(cJSON *json);
bool parsePattern(cJSON *json);
bool parseColour(cJSON *json);

void initAppState()
{
	appStateQueueHandle = xQueueCreate(2, sizeof(appStateChange_t));
	animChangedQueueHandle = xQueueCreate(2, sizeof(uint8_t));
	ESP_ERROR_CHECK(nvs_open("wp_appstate", NVS_READWRITE, &configHandle));

	uint8_t temp = displayPattern;
	if (nvs_get_u8(configHandle, KEY_DISPLAY_PATTERN, &temp) == ESP_OK)
		displayPattern = temp;
	if (nvs_get_u8(configHandle, KEY_SOUND_ON, &temp) == ESP_OK)
		soundOn = temp;
	if (nvs_get_u8(configHandle, KEY_CUSTOM_COLOUR_ACTIVE, &temp) == ESP_OK)
		customColourActive = temp;
	colour_t savedColour;
	size_t colourSize = sizeof(colour_t);
	if (nvs_get_blob(configHandle, KEY_CUSTOM_COLOUR, &savedColour, &colourSize) == ESP_OK)
		requestedColour = savedColour;

	xTaskCreate(&appStateTask, "appstate", 3192, NULL, 6, NULL);
}

void appStateTask(void *pvParameter)
{
	appStateChange_t msg;

	while (true)
	{
		if (pdTRUE == xQueueReceive(appStateQueueHandle, &msg, (portTickType)portMAX_DELAY))
		{
			parseStateJson(msg.jsonStr);
			free(msg.jsonStr);
		}
	}
}

void parseStateJson(const char *jsonStr)
{
	cJSON *json = cJSON_Parse(jsonStr);
	if (json == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			ESP_LOGE(APP_TAG, "Error before: %s\n", error_ptr);
		}
		return;
	}

	ESP_LOGI(APP_TAG, "Updated app state. Sound: %i, Anim: %i", soundOn, displayPattern);

	// Parse data, if NVS changed log for commit later
	bool nvsChanged = false;
	if (parseSound(json))
		nvsChanged = true;
	if (parsePattern(json))
		nvsChanged = true;
	if (parseColour(json))
		nvsChanged = true;

	if (!nvsChanged)
		return;

	// Commit NVS changes
	esp_err_t err = nvs_commit(configHandle);
	if (err != ESP_OK)
	{
		ESP_LOGW(APP_TAG, "Could not commit new app state");
	}
}

bool parseSound(cJSON *json)
{
	bool newSound = soundOn;

	const cJSON *val = cJSON_GetObjectItemCaseSensitive(json, "sound");
	if (cJSON_IsBool(val))
		newSound = cJSON_IsTrue(val);

	if (soundOn == newSound)
		return false;

	soundOn = newSound;
	uint8_t sound = soundOn;
	esp_err_t err = nvs_set_u8(configHandle, KEY_SOUND_ON, sound);
	if (err != ESP_OK)
	{
		ESP_LOGW(APP_TAG, "Could not save new sound request");
		return false;
	}

	return true;
}

bool parsePattern(cJSON *json)
{
	uint8_t newPattern = displayPattern;

	const cJSON *val = cJSON_GetObjectItemCaseSensitive(json, "anim");
	if (cJSON_IsNumber(val))
		newPattern = val->valueint;

	if (displayPattern == newPattern)
		return false;

	displayPattern = newPattern;
	xQueueSend(animChangedQueueHandle, &newPattern, 100 / portTICK_PERIOD_MS);
	esp_err_t err = nvs_set_u8(configHandle, KEY_DISPLAY_PATTERN, newPattern);
	if (err != ESP_OK)
	{
		ESP_LOGW(APP_TAG, "Could not save new pattern request");
		return false;
	}

	return true;
}

bool parseColour(cJSON *json)
{
	colour_t newColour = requestedColour;
	bool newColourActive = customColourActive;

	const cJSON *val = cJSON_GetObjectItemCaseSensitive(json, "colour");
	if (cJSON_IsBool(val))
		newColourActive = cJSON_IsTrue(val);

	val = cJSON_GetObjectItemCaseSensitive(json, "r");
	if (cJSON_IsNumber(val))
		newColour.r = val->valueint;
	val = cJSON_GetObjectItemCaseSensitive(json, "g");
	if (cJSON_IsNumber(val))
		newColour.g = val->valueint;
	val = cJSON_GetObjectItemCaseSensitive(json, "b");
	if (cJSON_IsNumber(val))
		newColour.b = val->valueint;

	ESP_LOGI(APP_TAG, "Active: %i, R: %i, G: %i, B: %i", newColourActive ? 1 : 0, newColour.r, newColour.g, newColour.b);

	if (newColourActive == customColourActive &&
		newColour.r == requestedColour.r &&
		newColour.g == requestedColour.g &&
		newColour.b == requestedColour.b)
		return false;

	requestedColour = newColour;
	customColourActive = newColourActive;

	esp_err_t err = nvs_set_u8(configHandle, KEY_CUSTOM_COLOUR_ACTIVE, newColourActive);
	if (err != ESP_OK)
	{
		ESP_LOGW(APP_TAG, "Could not save new custom colour activation state");
		return false;
	}

	err = nvs_set_blob(configHandle, KEY_CUSTOM_COLOUR, (void*)&newColour, sizeof(colour_t));
	if (err != ESP_OK)
	{
		ESP_LOGW(APP_TAG, "Could not save new custom colour");
		return false;
	}

	return true;
}
