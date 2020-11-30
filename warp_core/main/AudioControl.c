/*
 * AudioControl.c
 *
 *  Created on: 3 Mar 2018
 *      Author: dancl
 */

#include "AudioControl.h"
#include "Config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

uint8_t uartRxBuffer[256];

void initAudioControl()
{
	uart_config_t uart_config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity    = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};
	uart_param_config(UART_NUM_1, &uart_config);
	uart_set_pin(UART_NUM_1, PIN_UART_TX, PIN_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_driver_install(UART_NUM_1, 256 * 2, 0, 0, NULL, 0);

	// No TX buffer so that writing blocks until send complete to aid in synchronisation
}

void playAudioFile(uint8_t file)
{
	uart_write_bytes(UART_NUM_1, (const char *)&file, 1);

	// Remote takes a little time to start playing, so delay to give it the time
	vTaskDelay(170 / portTICK_PERIOD_MS);
}
