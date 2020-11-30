/*
 * StackAnimation.cpp
 *
 *  Created on: 9 Mar 2018
 *      Author: dancl
 */

#include "StackAnimation.h"
#include "esp_system.h"
#include "ApplicationState.h"

#define STATE_STACK_UP		(0)
#define STATE_STACK_DOWN	(1)
#define DELAY_MS			(40)

StackAnimation::StackAnimation(WarpCoreVisual &_core)
	: core(_core)
{
	colour.r = (uint8_t)(esp_random() % 255);
	colour.g = (uint8_t)(esp_random() % 255);
	colour.b = (uint8_t)(esp_random() % 255);
}

StackAnimation::~StackAnimation()
{
}

void StackAnimation::start()
{
	state = STATE_STACK_UP;

	currentPosition = 0;
	destinationRow = core.getNumRows() - 1;
}

void StackAnimation::end()
{
}

uint16_t StackAnimation::animate()
{
	if (state == STATE_STACK_UP)
		stackUp();
	else
		stackDown();

	core.display();

	return DELAY_MS;
}

void StackAnimation::stackUp()
{
	led_t displayColour = colour;
	if (customColourActive)
	{
		displayColour.r = requestedColour.r;
		displayColour.g = requestedColour.g;
		displayColour.b = requestedColour.b;
	}

	core.clear();
	uint8_t numRows = core.getNumRows();

	if (currentPosition < destinationRow)
		core.setRow(currentPosition++, displayColour.r, displayColour.g, displayColour.b);
	else
	{
		if (--destinationRow == 0) // Reached the top, start dropping off rows
		{
			state = STATE_STACK_DOWN;
			currentPosition = core.getNumRows() - 1;
			destinationRow = currentPosition;
			core.clear(displayColour.r, displayColour.g, displayColour.b);
			return;
		}
		else
			currentPosition = 0;
	}

	for (uint8_t i = destinationRow + 1; i < numRows; ++i)
		core.setRow(i, displayColour.r, displayColour.g, displayColour.b);
}

void StackAnimation::stackDown()
{
	led_t displayColour = colour;
	if (customColourActive)
	{
		displayColour.r = requestedColour.r;
		displayColour.g = requestedColour.g;
		displayColour.b = requestedColour.b;
	}

	core.clear();
	uint8_t numRows = core.getNumRows();

	if (currentPosition < numRows)
		core.setRow(currentPosition++, displayColour.r, displayColour.g, displayColour.b);
	else
	{
		if (--destinationRow == -1)
		{
			state = STATE_STACK_UP;
			currentPosition = 0;
			destinationRow = core.getNumRows() - 1;
			colour.r = (uint8_t)(esp_random() % 255);
			colour.g = (uint8_t)(esp_random() % 255);
			colour.b = (uint8_t)(esp_random() % 255);
			return;
		}
		else
		{
			core.setRow(destinationRow, displayColour.r, displayColour.g, displayColour.b);
			currentPosition = destinationRow;
		}
	}

	for (uint8_t i = 0; i < destinationRow; ++i)
		core.setRow(i, displayColour.r, displayColour.g, displayColour.b);
}

