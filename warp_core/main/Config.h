/*
 * Config.h
 *
 *  Created on: 22 Feb 2018
 *      Author: dancl
 */

#ifndef MAIN_CONFIG_H_
#define MAIN_CONFIG_H_

#define PIN_ROW_1	GPIO_NUM_18
#define PIN_ROW_2	GPIO_NUM_23
#define PIN_ROW_3	GPIO_NUM_19
#define PIN_ROW_4	GPIO_NUM_21
#define PIN_ROW_5	GPIO_NUM_22

#define MAX_CHANNEL_BRIGHTNESS	(128)
#define MAX_ALL_BRIGHTNESS		(128 * 2)
#define NUM_SECTIONS			(5)
#define LED_COUNT_ROW			(7)
#define LED_ROWS_PER_SECTION	(5)
#define LED_COUNT_CHAMBER		(14)

/*#define PIN_SCL			(19)
#define PIN_SDA			(23)
#define I2C_FREQ		(800000) // 800Khz
#define I2C_ADDR		(0x28)*/

#define PIN_UART_TX		(16)
#define PIN_UART_RX		(17)

#define DEVICE_IP			"192.168.1.35"
#define DEVICE_GW			"192.168.1.1"
#define DEVICE_NETMASK		"255.255.255.0"
#define DEVICE_DNS			"192.168.1.1"
#define DEVICE_DNS2			"8.8.8.8"
#define AP_TARGET_SSID		"X"
#define AP_TARGET_PASSWORD	"X"

#define MQTT_URI		"X"
#define MQTT_CLIENTID	"Core1"
#define MQTT_USERNAME	"X"
#define MQTT_PASSWORD	"X"
#define MQTT_DEVICE_SUB	"devices/Core1/messages/devicebound/#"
#define IOT_CONN_STR	"X"

#endif /* MAIN_CONFIG_H_ */
