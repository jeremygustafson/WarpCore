/*
 * wifi.c
 *
 *  Created on: 27 Feb 2018
 *      Author: dancl
 */

#include "WiFi.h"
#include "Config.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "lwip/sockets.h"

const char *WIFI_TAG = "WiFi";
EventGroupHandle_t wifi_event_group;

esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
	switch (event->event_id)
	{
	case SYSTEM_EVENT_STA_START:
		ESP_LOGI(WIFI_TAG, "SYSTEM_EVENT_STA_START");
		ESP_ERROR_CHECK(esp_wifi_connect());
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		ESP_LOGI(WIFI_TAG, "SYSTEM_EVENT_STA_GOT_IP");
		ESP_LOGI(WIFI_TAG, "got ip:%s\n",
		ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
		xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		ESP_LOGI(WIFI_TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
		xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
		ESP_ERROR_CHECK(esp_wifi_connect());
		break;
	default:
		break;
	}

	return ESP_OK;
}

void initWifi()
{
	wifi_event_group = xEventGroupCreate();

	tcpip_adapter_init();

	tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA); // Don't run a DHCP client
	tcpip_adapter_ip_info_t ipInfo;

	inet_pton(AF_INET, DEVICE_IP, &ipInfo.ip);
	inet_pton(AF_INET, DEVICE_GW, &ipInfo.gw);
	inet_pton(AF_INET, DEVICE_NETMASK, &ipInfo.netmask);
	tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);

	tcpip_adapter_dns_info_t dnsInfo;
	inet_pton(AF_INET, DEVICE_DNS, &dnsInfo.ip);
	ESP_ERROR_CHECK(tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_STA, TCPIP_ADAPTER_DNS_MAIN, &dnsInfo));
	inet_pton(AF_INET, DEVICE_DNS2, &dnsInfo.ip);
	ESP_ERROR_CHECK(tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_STA, TCPIP_ADAPTER_DNS_BACKUP, &dnsInfo));

	ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	wifi_config_t sta_config = {
	    .sta = {
	      .ssid      = AP_TARGET_SSID,
	      .password  = AP_TARGET_PASSWORD,
	      .bssid_set = 0,
		  .scan_method = WIFI_ALL_CHANNEL_SCAN
	    }
	};
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
	ESP_ERROR_CHECK(esp_wifi_start());
	esp_wifi_set_ps(WIFI_PS_NONE);
}

