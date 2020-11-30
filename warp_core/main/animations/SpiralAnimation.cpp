/*
 * SpiralAnimation.cpp
 *
 *  Created on: 9 Mar 2018
 *      Author: dancl
 */

#include "SpiralAnimation.h"
#include "colour.h"
#include "ApplicationState.h"

#define DELAY_MS	100

#define STATE_ANIM_IN	(0)
#define STATE_ROTATE	(1)
#define NUM_RAINBOWS	(1)

SpiralAnimation::SpiralAnimation(WarpCoreVisual &_core)
	: core(_core)
{
}

SpiralAnimation::~SpiralAnimation()
{
	if (rowStates != NULL)
	{
		delete[] rowStates;
		rowStates = NULL;
	}
}

void SpiralAnimation::start()
{
	state = 0;
	colourPosition = 0;
	colourLadder = 0;
	endRow = 0;
	numRows = core.getNumRows();
	numCols = core.getNumCols();

	if (rowStates != NULL)
		memset(rowStates, 0, numRows);
	else
		rowStates = new uint8_t[numRows];
}

void SpiralAnimation::end()
{
	delete[] rowStates;
	rowStates = NULL;
}

uint16_t SpiralAnimation::animate()
{
	core.clear();

	if (state == STATE_ANIM_IN)
		animateIn();
	else if (state == STATE_ROTATE)
		animateRotation();

	float multi = (255.0f / (float)NUM_RAINBOWS) / (float)numRows;
	float floatColourPosition = (float)colourPosition;

	for (uint8_t i = 0; i <= endRow; ++i)
	{
		uint16_t nominalPosition = floatColourPosition + ((float)(i + colourLadder) * multi);
		led_t colour = getPixelColour(nominalPosition);
		core.setLed(i, rowStates[i], colour.r, colour.g, colour.b);
	}

	core.display();

	return DELAY_MS;
}

void SpiralAnimation::animateIn()
{
	uint8_t col = 0;

	// Find last column used, then add one to it
	if (endRow > 0)
		col = rowStates[endRow - 1] + 1;

	if (col + 1 >= numCols)
		col = 0;

	rowStates[endRow++] = col;
	if (endRow >= numRows)
	{
		state = STATE_ROTATE;
		endRow = numRows - 1;
	}
}

void SpiralAnimation::animateRotation()
{
	for (uint8_t i = 0; i < numRows; ++i)
	{
		uint8_t newCol = rowStates[i] + 1;
		if (newCol >= numCols)
			newCol = 0;

		rowStates[i] = newCol;
	}

	++colourPosition;
	if (++colourLadder > numRows)
		colourLadder = 0;
}

led_t SpiralAnimation::getPixelColour(uint16_t position)
{
	if (!customColourActive)
		return getRgbColour(position % 255);

	float nominalPosition = (position % 255) / 255.0f;
	float r = requestedColour.r * nominalPosition;
	float g = requestedColour.g * nominalPosition;
	float b = requestedColour.b * nominalPosition;

	return led_t(r, g, b);
}
