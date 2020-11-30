/*
 * WebServer.cpp
 *
 *  Created on: 1 Mar 2018
 *      Author: dancl
 */

#include "WebServer.h"

#include <string>
#include <FS.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "ApplicationState.h"

const char *WEB_TAG = "WebServer";
AsyncWebServer server(80);

void logNotFound(AsyncWebServerRequest *request)
{
	ESP_LOGI(WEB_TAG, "NOT_FOUND: ");
	if (request->method() == HTTP_GET)
		ESP_LOGI(WEB_TAG, "GET");
	else if (request->method() == HTTP_POST)
		ESP_LOGI(WEB_TAG, "POST");
	else if (request->method() == HTTP_DELETE)
		ESP_LOGI(WEB_TAG, "DELETE");
	else if (request->method() == HTTP_PUT)
		ESP_LOGI(WEB_TAG, "PUT");
	else if (request->method() == HTTP_PATCH)
		ESP_LOGI(WEB_TAG, "PATCH");
	else if (request->method() == HTTP_HEAD)
		ESP_LOGI(WEB_TAG, "HEAD");
	else if (request->method() == HTTP_OPTIONS)
		ESP_LOGI(WEB_TAG, "OPTIONS");
	else
		ESP_LOGI(WEB_TAG, "UNKNOWN");
	ESP_LOGI(WEB_TAG, " http://%s%s\n", request->host().c_str(), request->url().c_str());

	if (request->contentLength())
	{
		ESP_LOGI(WEB_TAG, "_CONTENT_TYPE: %s\n", request->contentType().c_str());
		ESP_LOGI(WEB_TAG, "_CONTENT_LENGTH: %u\n", request->contentLength());
	}

	int headers = request->headers();
	for(int i = 0; i < headers; ++i)
	{
		AsyncWebHeader* h = request->getHeader(i);
		ESP_LOGI(WEB_TAG, "_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
	}

	int params = request->params();
	for(int i = 0; i < params; ++i)
	{
		AsyncWebParameter* p = request->getParam(i);
		if(p->isFile())
			ESP_LOGI(WEB_TAG, "_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
		else if(p->isPost())
			ESP_LOGI(WEB_TAG, "_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
		else
			ESP_LOGI(WEB_TAG, "_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
	}

	request->send(404);
}

void handleUpdateState(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
	ESP_LOGI(WEB_TAG, "Update command received");

	// Copy data to our own buffer
	char *json = (char *)malloc(len + 1);
	memcpy(json, data, len);
	json[len] = 0;

	// Send JSON for parsing & action
	appStateChange_t msg = {};
	msg.jsonStr = json;
	if (xQueueSend(appStateQueueHandle, &msg, 10 / portTICK_RATE_MS) != pdTRUE)
	{
		ESP_LOGW(WEB_TAG, "Could not enqueue state change");
		free(json);
	}
}

void initWebServer()
{
	SPIFFS.begin();

	server.on("/api/updatestate", HTTP_POST,
			[](AsyncWebServerRequest *request)
			{
				ESP_LOGI(WEB_TAG, "Update request complete");
				request->send(200, "text/plain", "OK");
			}, NULL, handleUpdateState);
	server.serveStatic("/", SPIFFS, "/web/").setDefaultFile("index.htm");
	server.onNotFound(logNotFound);

	server.begin();
}
