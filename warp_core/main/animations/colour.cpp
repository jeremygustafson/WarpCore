/*
 * colour.c
 *
 *  Created on: 9 Mar 2018
 *      Author: dancl
 */

#include "colour.h"

led_t getRgbColour(uint8_t wheelPos)
{
	uint8_t pos = 255 - wheelPos;
	if (pos < 85)
	{
		led_t led(255 - pos * 3, 0, pos * 3);
		return led;
	}
	if (pos < 170)
	{
		pos -= 85;
		led_t led(0, pos * 3, 255 - pos * 3);
		return led;
	}
	pos -= 170;
	{
		led_t led(pos * 3, 255 - pos * 3, 0);
		return led;
	}
}
