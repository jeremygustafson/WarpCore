#include "BtAudio.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

#include "BtAppCore.h"

#include "AudioFiles.h"

#define BT_APP_HEART_BEAT_EVT                (0xff00)
const char *BT_TAG = "BT-Audio";

/// Initialise the A2DP source device
static void initSource();

/// callback for GAP events
void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);

/// callback function for A2DP source
static void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);

/// callback function for A2DP source audio data stream
static int32_t bt_app_a2d_data_cb(uint8_t *data, int32_t len);

static void a2d_app_heart_beat(void *arg);

/// A2DP application state machine
static void bt_app_av_sm_hdlr(uint16_t event, void *param);

/* A2DP application state machine handler for each state */
static void bt_app_av_state_unconnected(uint16_t event, void *param);
static void bt_app_av_state_connecting(uint16_t event, void *param);
static void bt_app_av_state_connected(uint16_t event, void *param);
static void bt_app_av_state_disconnecting(uint16_t event, void *param);

static esp_bd_addr_t peer_bda =
{ 0 };
static uint8_t peer_bdname[ESP_BT_GAP_MAX_BDNAME_LEN + 1];
static int m_a2d_state = APP_AV_STATE_IDLE;
static int m_media_state = APP_AV_MEDIA_STATE_IDLE;
static int m_connecting_intv = 0;
static uint32_t m_pkt_cnt = 0;
static uint32_t m_scanDelayTicks = 0;

EventGroupHandle_t btEventGroup;

typedef struct audioPlaySpec
{
	FILE *audioFile;
	fpos_t dataOffset;
	uint32_t currentRepeat;
	uint32_t repeatCount;
	uint32_t repeatDelay;
	bool inDelay;
	uint32_t delayStart;
} audioPlaySpec;

SemaphoreHandle_t playbackLock;
audioPlaySpec playbackFile = {};
TimerHandle_t heartbeatTimer;

void initBtAudio(const char *deviceName)
{
	playbackLock = xSemaphoreCreateMutex();
	assert(playbackLock != NULL);
	btEventGroup = xEventGroupCreate();
	assert(btEventGroup != NULL);
	xEventGroupSetBits(btEventGroup, BIT_BT_STOPPED);
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

	int ret = esp_bt_controller_init(&bt_cfg);
	if (ret != ESP_OK)
	//if (esp_bt_controller_init(&bt_cfg) != ESP_OK)
	{
		ESP_LOGE(BT_TAG, "%s initialize controller failed\n", __func__);
		return;
	}

	ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
	if (ret != ESP_OK)
	//if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK)
	{
		ESP_LOGE(BT_TAG, "%s enable controller failed\n", __func__);
		return;
	}

	if (esp_bluedroid_init() != ESP_OK)
	{
		ESP_LOGE(BT_TAG, "%s initialize bluedroid failed\n", __func__);
		return;
	}

	if (esp_bluedroid_enable() != ESP_OK)
	{
		ESP_LOGE(BT_TAG, "%s enable bluedroid failed\n", __func__);
		return;
	}

	bt_app_task_start_up();
	initSource(deviceName);
}

void playBtAudioFile(FILE *file, fpos_t dataStart, uint32_t repeatCount, uint32_t repeatDelay)
{
	if (xSemaphoreTake(playbackLock, 5000 / portTICK_PERIOD_MS) != pdTRUE)
	{
		ESP_LOGE(BT_TAG, "Could not get lock on playback file to play new file");
		return;
	}

	playbackFile.audioFile = file;
	playbackFile.dataOffset = dataStart;
	playbackFile.currentRepeat = 0;
	playbackFile.repeatCount = repeatCount;
	playbackFile.repeatDelay = repeatDelay;
	playbackFile.inDelay = false;
	playbackFile.delayStart = 0;

	xSemaphoreGive(playbackLock);
}

static void initSource(const char *deviceName)
{
	/* set up device name */
	esp_bt_dev_set_device_name(deviceName);

	/* register GAP callback function */
	esp_bt_gap_register_callback(bt_app_gap_cb);

	/* initialize A2DP source */
	esp_a2d_register_callback(&bt_app_a2d_cb);
	esp_a2d_source_register_data_callback(bt_app_a2d_data_cb);
	esp_a2d_source_init();

	/* set connectable mode */
	esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE);

	/* start device discovery */
	ESP_LOGI(BT_TAG, "Starting device discovery...");
	m_a2d_state = APP_AV_STATE_DISCOVERING;
	esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);

	/* create and start heart beat timer */
	int tmr_id = 0;
	heartbeatTimer = xTimerCreate("connTmr", (10000 / portTICK_RATE_MS), pdTRUE,
			(void *) tmr_id, a2d_app_heart_beat);
	xTimerStart(heartbeatTimer, portMAX_DELAY);
}

static char *bda2str(esp_bd_addr_t bda, char *str, size_t size)
{
	if (bda == NULL || str == NULL || size < 18)
	{
		return NULL;
	}

	uint8_t *p = bda;
	sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", p[0], p[1], p[2], p[3], p[4],
			p[5]);
	return str;
}

static bool get_name_from_eir(uint8_t *eir, uint8_t *bdname,
		uint8_t *bdname_len)
{
	uint8_t *rmt_bdname = NULL;
	uint8_t rmt_bdname_len = 0;

	if (!eir)
	{
		return false;
	}

	rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &rmt_bdname_len);
	if (!rmt_bdname)
		rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &rmt_bdname_len);

	if (rmt_bdname)
	{
		if (rmt_bdname_len > ESP_BT_GAP_MAX_BDNAME_LEN)
			rmt_bdname_len = ESP_BT_GAP_MAX_BDNAME_LEN;

		if (bdname)
		{
			memcpy(bdname, rmt_bdname, rmt_bdname_len);
			bdname[rmt_bdname_len] = '\0';
		}
		if (bdname_len)
			*bdname_len = rmt_bdname_len;
		return true;
	}

	return false;
}

static void filter_inquiry_scan_result(esp_bt_gap_cb_param_t *param)
{
	char bda_str[18];
	uint32_t cod = 0;
	int32_t rssi = -129; /* invalid value */
	uint8_t *eir = NULL;
	esp_bt_gap_dev_prop_t *p;

	ESP_LOGI(BT_TAG, "Scanned device: %s", bda2str(param->disc_res.bda, bda_str, 18));
	for (int i = 0; i < param->disc_res.num_prop; i++)
	{
		p = param->disc_res.prop + i;
		switch (p->type)
		{
		case ESP_BT_GAP_DEV_PROP_COD:
			cod = *(uint32_t *) (p->val);
			ESP_LOGI(BT_TAG, "--Class of Device: 0x%x", cod);
			break;
		case ESP_BT_GAP_DEV_PROP_RSSI:
			rssi = *(int8_t *) (p->val);
			ESP_LOGI(BT_TAG, "--RSSI: %d", rssi);
			break;
		case ESP_BT_GAP_DEV_PROP_EIR:
			eir = (uint8_t *) (p->val);
			break;
		case ESP_BT_GAP_DEV_PROP_BDNAME:
		default:
			break;
		}
	}

	/* search for device with MAJOR service class as "rendering" in COD */
	if (!esp_bt_gap_is_valid_cod(cod)
			|| !(esp_bt_gap_get_cod_srvc(cod) & ESP_BT_COD_SRVC_RENDERING))
	{
		ESP_LOGI(BT_TAG, "No devices found with required service class");
		return;
	}

	/* search for device named "ESP_SPEAKER" in its extended inqury response */
	//if (eir)
	//{
		get_name_from_eir(eir, peer_bdname, NULL);
		ESP_LOGI(BT_TAG, "Found a target device, address %s, name %s",
				bda_str, peer_bdname);
		m_a2d_state = APP_AV_STATE_DISCOVERED;
		memcpy(peer_bda, param->disc_res.bda, ESP_BD_ADDR_LEN);
		ESP_LOGI(BT_TAG, "Cancel device discovery ...");
		esp_bt_gap_cancel_discovery();
	//}
}

void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
	ESP_LOGI(BT_TAG, "GAP Callback: 0x%x", event);

	switch (event)
	{
	case ESP_BT_GAP_DISC_RES_EVT:
	{
		filter_inquiry_scan_result(param);
		break;
	}
	case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
	{
		if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED)
		{
			if (m_a2d_state == APP_AV_STATE_DISCOVERED)
			{
				m_a2d_state = APP_AV_STATE_CONNECTING;
				ESP_LOGI(BT_TAG, "Device discovery stopped.");
				ESP_LOGI(BT_TAG, "a2dp connecting to peer: %s", peer_bdname);
				esp_a2d_source_connect(peer_bda);
			}
			else
			{
				// not discovered, continue to discover
				ESP_LOGI(BT_TAG, "Device discovery failed, sleeping pending discover");
				m_a2d_state = APP_AV_STATE_PENDING_DISCOVERING;
				m_scanDelayTicks = (xTaskGetTickCount() * portTICK_PERIOD_MS) + (BT_DISCOVERY_DELAY_MS / portTICK_PERIOD_MS);
			}
		}
		else if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED)
		{
			ESP_LOGI(BT_TAG, "Discovery started.");
		}
		break;
	}
	case ESP_BT_GAP_RMT_SRVCS_EVT:
	case ESP_BT_GAP_RMT_SRVC_REC_EVT:
	default:
	{
		ESP_LOGI(BT_TAG, "event: %d", event);
		break;
	}
	}
	return;
}

static void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
	bt_app_work_dispatch(bt_app_av_sm_hdlr, event, param,
			sizeof(esp_a2d_cb_param_t), NULL);
}

static int32_t bt_app_a2d_data_cb(uint8_t *data, int32_t len)
{
	if (len < 0 || data == NULL)
		return 0;

	// Get lock on playback, only wait 0.1ms
	if (xSemaphoreTake(playbackLock, portTICK_PERIOD_MS / 100) != pdTRUE)
	{
		xEventGroupClearBits(btEventGroup, BIT_BT_PLAYING);
		xEventGroupSetBits(btEventGroup, BIT_BT_STOPPED);
		ESP_LOGE(BT_TAG, "Could not get lock on playback file, sending silence");
		memset(data, 0, len);
		return len;
	}

	// If we're at the end of the repeat count
	// Or still in a delay, just send silence
	if ((playbackFile.currentRepeat > playbackFile.repeatCount) ||
		(playbackFile.inDelay && (xTaskGetTickCount() * portTICK_PERIOD_MS) - playbackFile.delayStart < playbackFile.repeatDelay) ||
		 playbackFile.audioFile == NULL)
	{
		if (playbackFile.currentRepeat > playbackFile.repeatCount)
		{
			if (playbackFile.audioFile != NULL)
			{
				fclose(playbackFile.audioFile);
				playbackFile.audioFile = NULL;
			}
			xEventGroupClearBits(btEventGroup, BIT_BT_PLAYING);
			xEventGroupSetBits(btEventGroup, BIT_BT_STOPPED);
		}

		xSemaphoreGive(playbackLock);
		memset(data, 0, len);
		return len;
	}

	xEventGroupSetBits(btEventGroup, BIT_BT_PLAYING);
	xEventGroupClearBits(btEventGroup, BIT_BT_STOPPED);
	playbackFile.inDelay = false;
	size_t dataRead = fread(data, 1, len, playbackFile.audioFile);

	// Audio filled buffer entirely, send directly
	if (dataRead == len)
	{
		xSemaphoreGive(playbackLock);
		return len;
	}

	// At end of file, copy what's left and then continue with silence
	// Reset file data position back to the start
	size_t remaining = len - dataRead;
	++playbackFile.currentRepeat;
	playbackFile.delayStart = xTaskGetTickCount() * portTICK_PERIOD_MS;
	playbackFile.inDelay = true;
	fsetpos(playbackFile.audioFile, &playbackFile.dataOffset);
	xSemaphoreGive(playbackLock);
	memset(data + remaining, 0, len - remaining);
	return len;
}

static void a2d_app_heart_beat(void *arg)
{
	bt_app_work_dispatch(bt_app_av_sm_hdlr, BT_APP_HEART_BEAT_EVT, NULL, 0, NULL);
}

static void bt_app_av_sm_hdlr(uint16_t event, void *param)
{
	ESP_LOGI(BT_TAG, "%s state %d, evt 0x%x", __func__, m_a2d_state, event);
	switch (m_a2d_state)
	{
	case APP_AV_STATE_DISCOVERING:
	case APP_AV_STATE_DISCOVERED:
		break;
	case APP_AV_STATE_PENDING_DISCOVERING:
		if (xTaskGetTickCount() * portTICK_PERIOD_MS >= m_scanDelayTicks)
		{
			ESP_LOGI(BT_TAG, "Resuming scan for devices");
			m_a2d_state = APP_AV_STATE_DISCOVERING;
			esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10,	0);
		}
		break;
	case APP_AV_STATE_UNCONNECTED:
		bt_app_av_state_unconnected(event, param);
		break;
	case APP_AV_STATE_CONNECTING:
		bt_app_av_state_connecting(event, param);
		break;
	case APP_AV_STATE_CONNECTED:
		bt_app_av_state_connected(event, param);
		break;
	case APP_AV_STATE_DISCONNECTING:
		bt_app_av_state_disconnecting(event, param);
		break;
	default:
		ESP_LOGE(BT_TAG, "%s invalid state %d", __func__, m_a2d_state);
		break;
	}
}

static void bt_app_av_state_unconnected(uint16_t event, void *param)
{
	xEventGroupClearBits(btEventGroup, BIT_BT_AUDIO_READY | BIT_BT_PLAYING);
	xEventGroupSetBits(btEventGroup, BIT_BT_STOPPED);

	switch (event)
	{
	case ESP_A2D_CONNECTION_STATE_EVT:
		m_a2d_state = APP_AV_STATE_DISCOVERING;
		esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE);
		esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
		break;
	case ESP_A2D_AUDIO_STATE_EVT:
	case ESP_A2D_AUDIO_CFG_EVT:
	case ESP_A2D_MEDIA_CTRL_ACK_EVT:
		break;
	case BT_APP_HEART_BEAT_EVT:
	{
		uint8_t *p = peer_bda;
		ESP_LOGI(BT_TAG,
				"a2dp connecting to peer: %02x:%02x:%02x:%02x:%02x:%02x", p[0],
				p[1], p[2], p[3], p[4], p[5]);
		esp_a2d_source_connect(peer_bda);
		m_a2d_state = APP_AV_STATE_CONNECTING;
		m_connecting_intv = 0;
		break;
	}
	default:
		ESP_LOGE(BT_TAG, "%s unhandled evt %d", __func__, event);
		break;
	}
}

static void bt_app_av_state_connecting(uint16_t event, void *param)
{
	ESP_LOGI(BT_TAG, "%s event %d", __func__, event);

	esp_a2d_cb_param_t *a2d = NULL;
	switch (event)
	{
	case ESP_A2D_CONNECTION_STATE_EVT:
	{
		a2d = (esp_a2d_cb_param_t *) (param);
		if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED)
		{
			ESP_LOGI(BT_TAG, "a2dp connected");
			m_a2d_state = APP_AV_STATE_CONNECTED;
			m_media_state = APP_AV_MEDIA_STATE_IDLE;

		}
		else if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED)
		{
			m_a2d_state = APP_AV_STATE_UNCONNECTED;
			m_a2d_state = APP_AV_STATE_DISCOVERING;
			esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE);
			esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
		}
		break;
	}
	case ESP_A2D_AUDIO_STATE_EVT:
	case ESP_A2D_AUDIO_CFG_EVT:
	case ESP_A2D_MEDIA_CTRL_ACK_EVT:
		break;
	case BT_APP_HEART_BEAT_EVT:
		if (++m_connecting_intv >= 2)
		{
			m_a2d_state = APP_AV_STATE_UNCONNECTED;
			m_connecting_intv = 0;
		}
		break;
	default:
		ESP_LOGE(BT_TAG, "%s unhandled evt %d", __func__, event);
		break;
	}
}

static void bt_app_av_media_proc(uint16_t event, void *param)
{
	esp_a2d_cb_param_t *a2d = NULL;
	switch (m_media_state)
	{
	case APP_AV_MEDIA_STATE_IDLE:
	{
		if (event == BT_APP_HEART_BEAT_EVT)
		{
			ESP_LOGI(BT_TAG, "a2dp media ready checking ...");
			esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_CHECK_SRC_RDY);
		}
		else if (event == ESP_A2D_MEDIA_CTRL_ACK_EVT)
		{
			a2d = (esp_a2d_cb_param_t *) (param);
			if (a2d->media_ctrl_stat.cmd == ESP_A2D_MEDIA_CTRL_CHECK_SRC_RDY
					&& a2d->media_ctrl_stat.status
							== ESP_A2D_MEDIA_CTRL_ACK_SUCCESS)
			{
				ESP_LOGI(BT_TAG, "a2dp media ready, starting ...");
				esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_START);
				m_media_state = APP_AV_MEDIA_STATE_STARTING;
			}
		}
		break;
	}
	case APP_AV_MEDIA_STATE_STARTING:
	{
		if (event == ESP_A2D_MEDIA_CTRL_ACK_EVT)
		{
			a2d = (esp_a2d_cb_param_t *) (param);
			if (a2d->media_ctrl_stat.cmd == ESP_A2D_MEDIA_CTRL_START
					&& a2d->media_ctrl_stat.status == ESP_A2D_MEDIA_CTRL_ACK_SUCCESS)
			{
				ESP_LOGI(BT_TAG, "a2dp media start successfully.");
				m_media_state = APP_AV_MEDIA_STATE_STARTED;
				xEventGroupSetBits(btEventGroup, BIT_BT_AUDIO_READY);
			}
			else
			{
				// not started succesfully, transfer to idle state
				ESP_LOGI(BT_TAG, "a2dp media start failed.");
				m_media_state = APP_AV_MEDIA_STATE_IDLE;
			}
		}
		break;
	}
	case APP_AV_MEDIA_STATE_STARTED:
	{
		if (event == ESP_A2D_MEDIA_CTRL_ACK_EVT)
		{
			a2d = (esp_a2d_cb_param_t *)(param);
			ESP_LOGI(BT_TAG, "Received command: %i", a2d->media_ctrl_stat.cmd);
		}
		/*if (event == BT_APP_HEART_BEAT_EVT)
		{
		}*/
		break;
	}
	case APP_AV_MEDIA_STATE_STOPPING:
	{
		xEventGroupClearBits(btEventGroup, BIT_BT_AUDIO_READY | BIT_BT_PLAYING);
		xEventGroupSetBits(btEventGroup, BIT_BT_STOPPED);

		if (event == ESP_A2D_MEDIA_CTRL_ACK_EVT)
		{
			a2d = (esp_a2d_cb_param_t *) (param);
			if (a2d->media_ctrl_stat.cmd == ESP_A2D_MEDIA_CTRL_STOP
					&& a2d->media_ctrl_stat.status ==  ESP_A2D_MEDIA_CTRL_ACK_SUCCESS)
			{
				ESP_LOGI(BT_TAG,
						"a2dp media stopped successfully, disconnecting...");
				m_media_state = APP_AV_MEDIA_STATE_IDLE;
				esp_a2d_source_disconnect(peer_bda);
				m_a2d_state = APP_AV_STATE_DISCONNECTING;
			}
			else
			{
				ESP_LOGI(BT_TAG, "a2dp media stopping...");
				esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_STOP);
			}
		}
		break;
	}
	}
}

static void bt_app_av_state_connected(uint16_t event, void *param)
{
	ESP_LOGI(BT_TAG, "%s evt %d", __func__, event);

	esp_a2d_cb_param_t *a2d = NULL;
	switch (event)
	{
	case ESP_A2D_CONNECTION_STATE_EVT:
	{
		a2d = (esp_a2d_cb_param_t *) (param);
		if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED)
		{
			ESP_LOGI(BT_TAG, "a2dp disconnected");
			m_a2d_state = APP_AV_STATE_UNCONNECTED;
		}
		break;
	}
	case ESP_A2D_AUDIO_STATE_EVT:
	{
		a2d = (esp_a2d_cb_param_t *) (param);
		if (ESP_A2D_AUDIO_STATE_STARTED == a2d->audio_stat.state)
		{
			m_pkt_cnt = 0;
		}
		break;
	}
	case ESP_A2D_AUDIO_CFG_EVT:
		// not suppposed to occur for A2DP source
		break;
	case ESP_A2D_MEDIA_CTRL_ACK_EVT:
	case BT_APP_HEART_BEAT_EVT:
	{
		bt_app_av_media_proc(event, param);
		break;
	}
	default:
		ESP_LOGE(BT_TAG, "%s unhandled evt %d", __func__, event);
		break;
	}
}

static void bt_app_av_state_disconnecting(uint16_t event, void *param)
{
	xEventGroupClearBits(btEventGroup, BIT_BT_AUDIO_READY);

	esp_a2d_cb_param_t *a2d = NULL;
	switch (event)
	{
	case ESP_A2D_CONNECTION_STATE_EVT:
	{
		a2d = (esp_a2d_cb_param_t *) (param);
		if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED)
		{
			ESP_LOGI(BT_TAG, "a2dp disconnected");
			m_a2d_state = APP_AV_STATE_UNCONNECTED;
		}
		break;
	}
	case ESP_A2D_AUDIO_STATE_EVT:
	case ESP_A2D_AUDIO_CFG_EVT:
	case ESP_A2D_MEDIA_CTRL_ACK_EVT:
	case BT_APP_HEART_BEAT_EVT:
		break;
	default:
		ESP_LOGE(BT_TAG, "%s unhandled evt %d", __func__, event);
		break;
	}
}
