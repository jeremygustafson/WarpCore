/*
 * SolidColourAnimation.cpp
 *
 *  Created on: 17 Mar 2018
 *      Author: dancl
 */

#include "SolidColourAnimation.h"
#include "ApplicationState.h"

#define DELAY_MS	(1000)

SolidColourAnimation::SolidColourAnimation(WarpCoreVisual &_core)
	: core(_core)
{
	colour.r = 0;
	colour.g = 0;
	colour.b = 255;
}

SolidColourAnimation::~SolidColourAnimation()
{
}

void SolidColourAnimation::start()
{
	lastColour = {};
}

void SolidColourAnimation::end()
{
}

uint16_t SolidColourAnimation::animate()
{
	if (customColourActive)
	{
		if (lastColour.r == requestedColour.r &&
			lastColour.g == requestedColour.g &&
			lastColour.b == requestedColour.b)
			return DELAY_MS;

		core.clear(requestedColour.r, requestedColour.g, requestedColour.b);
		lastColour.r = requestedColour.r;
		lastColour.g = requestedColour.g;
		lastColour.b = requestedColour.b;
	}
	else
	{
		if (lastColour.r == colour.r &&
			lastColour.g == colour.g &&
			lastColour.b == colour.b)
			return DELAY_MS;

		core.clear(colour.r, colour.g, colour.b);
		lastColour = colour;
	}

	core.display();
	return DELAY_MS;
}
