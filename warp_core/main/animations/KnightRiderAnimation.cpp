/*
 * KnightRiderAnimation.cpp
 *
 *  Created on: 17 Mar 2018
 *      Author: dancl
 */

#include "KnightRiderAnimation.h"
#include "ApplicationState.h"

#define DELAY_MS		(90)
#define ROW_INCREMENT	(1)
#define GLOW_LENGTH		(2)

#define STATE_DOWN		(0)
#define STATE_UP		(1)

KnightRiderAnimation::KnightRiderAnimation(WarpCoreVisual &_core)
: core(_core)
{
	dimSubtract = 255 / GLOW_LENGTH;
}

KnightRiderAnimation::~KnightRiderAnimation()
{
	if (rowColours != NULL)
	{
		delete[] rowColours;
		rowColours = NULL;
	}
}

void KnightRiderAnimation::start()
{
	state = STATE_DOWN;
	currentPosition = 0;
	numRows = core.getCentralRowIndex();

	max = numRows - 1;
	min = 0;
	float midPoint = (max + min) / 2.0f;
	range = max - midPoint;

	rowColours = new led_t[numRows];
	memset(rowColours, 0, sizeof(led_t) * numRows);
}

void KnightRiderAnimation::end()
{
	if (rowColours != NULL)
	{
		delete[] rowColours;
		rowColours = NULL;
	}
}

uint16_t KnightRiderAnimation::animate()
{
	dimTrail();
	if (currentPosition >= 0 && currentPosition < numRows)
		rowColours[currentPosition] = getColour();

	uint8_t totalRows = core.getNumRows();
	core.clear();
	for (uint8_t i = 0; i < numRows; ++i)
	{
		core.setRow(i, rowColours[i].r, rowColours[i].g, rowColours[i].b);
		core.setRow(totalRows - i - 1, rowColours[i].r, rowColours[i].g, rowColours[i].b);
	}
	core.display();

	float distanceFromCentre = abs(range - (currentPosition - min));
	float delayMs = (float)DELAY_MS * (distanceFromCentre / range);
	if (delayMs < 10.0f)
		delayMs = 10.0f;

	if (state == STATE_DOWN)
	{
		currentPosition += ROW_INCREMENT;
		if (currentPosition >= max)
			state = STATE_UP;
	}
	else
	{
		currentPosition -= ROW_INCREMENT;
		if (currentPosition <= min)
			state = STATE_DOWN;
	}

	return (uint16_t)delayMs;
}

void KnightRiderAnimation::dimTrail()
{
	for (uint8_t i = 0; i < numRows; ++i)
	{
		int16_t r = rowColours[i].r;
		int16_t g = rowColours[i].g;
		int16_t b = rowColours[i].b;

		if (r > 0)
		{
			r -= dimSubtract;
			if (r < 0)
				r = 0;
			rowColours[i].r = r;
		}
		if (g > 0)
		{
			g -= dimSubtract;
			if (g < 0)
				g = 0;
			rowColours[i].g = g;
		}
		if (b > 0)
		{
			b -= dimSubtract;
			if (b < 0)
				b = 0;
			rowColours[i].b = b;
		}
	}
}

led_t KnightRiderAnimation::getColour()
{
	led_t displayColour = {};

	if (customColourActive)
	{
		displayColour.r = requestedColour.r;
		displayColour.g = requestedColour.g;
		displayColour.b = requestedColour.b;
	}
	else
		displayColour.r = 255.0f;

	return displayColour;
}
