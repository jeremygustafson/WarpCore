/*
 * RainbowAnimation.cpp
 *
 *  Created on: 3 Mar 2018
 *      Author: dancl
 */

#include "RainbowAnimation.h"
#include "colour.h"

#define DELAY_TIME_FULL		(100)
#define DELAY_TIME_ROW		(80)
#define DELAY_TIME_COL		(40)
#define DELAY_TIME_BOTH		(40)

#define ROW_NUM_RAINBOWS	(5)
#define COL_NUM_RAINBOWS	(2)
#define BOTH_NUM_RAINBOWS	(3)

RainbowAnimation::RainbowAnimation(WarpCoreVisual &_core)
	: core(_core)
{
}

RainbowAnimation::~RainbowAnimation()
{
}

void RainbowAnimation::start()
{
}

void RainbowAnimation::end()
{
}

uint16_t RainbowAnimation::animate()
{
	switch (type)
	{
	case RAINBOW_FULL:
		return animateFull();

	case RAINBOW_CHASE_ROW:
		return animateRow();

	case RAINBOW_CHASE_COL:
		return animateCol();

	case RAINBOW_CHASE_BOTH:
		return animateBoth();
	}

	return DELAY_TIME_FULL;
}

uint16_t RainbowAnimation::animateFull()
{
	if (animationPosition > 255)
		animationPosition = animationPosition % 255;

	led_t colour = getRgbColour(animationPosition++);

	core.clear(colour.r, colour.g, colour.b);
	core.display();

	// Full anim is limited to 255 steps
	if (++animationPosition > 255)
		animationPosition = 0;

	return DELAY_TIME_FULL;
}

uint16_t RainbowAnimation::animateRow()
{
	uint16_t maxPosition = 255 * ROW_NUM_RAINBOWS;
	if (animationPosition > maxPosition)
		animationPosition = animationPosition % maxPosition;

	uint8_t numRows = core.getNumRows();
	float multi = (255.0f / (float)ROW_NUM_RAINBOWS) / numRows;
	float floatAnimPosition = (float)animationPosition;

	for (uint8_t i = 0; i < numRows; ++i)
	{
		uint16_t nominalPosition = floatAnimPosition + ((float)i * multi);
		led_t colour = getRgbColour(nominalPosition % 255);
		core.setRow(i, colour.r, colour.g, colour.b);
	}
	core.display();

	if (++animationPosition > maxPosition)
		animationPosition = 0;

	return DELAY_TIME_ROW;
}

uint16_t RainbowAnimation::animateCol()
{
	uint16_t maxPosition = 255 * COL_NUM_RAINBOWS;
	if (animationPosition > maxPosition)
		animationPosition = animationPosition % maxPosition;

	uint8_t numCols = core.getNumCols();
	float multi = (255.0f / (float)COL_NUM_RAINBOWS) / numCols;
	float floatAnimPosition = (float)animationPosition;

	for (uint8_t i = 0; i < numCols; ++i)
	{
		uint16_t nominalPosition = floatAnimPosition + ((float)i * multi);
		led_t colour = getRgbColour(nominalPosition % 255);
		core.setCol(i, colour.r, colour.g, colour.b, true);
	}
	core.display();

	if (++animationPosition > maxPosition)
		animationPosition = 0;

	return DELAY_TIME_COL;
}

uint16_t RainbowAnimation::animateBoth()
{
	uint16_t maxPosition = 255 * BOTH_NUM_RAINBOWS;
	if (animationPosition > maxPosition)
		animationPosition = animationPosition % maxPosition;

	uint8_t numCols = core.getNumCols();
	uint8_t numRows = core.getNumRows();
	float multi = (255.0f / (float)BOTH_NUM_RAINBOWS) / (numCols * numRows);
	float floatAnimPosition = (float)animationPosition;

	for (uint8_t row = 0; row < numRows; ++row)
	{
		for (uint8_t col = 0; col < numCols; ++col)
		{
			uint16_t nominalIndex = row * col;
			uint16_t nominalPosition = floatAnimPosition + ((float)nominalIndex * multi);
			led_t colour = getRgbColour(nominalPosition % 255);
			core.setLed(row, col, colour.r, colour.g, colour.b);
		}
	}
	core.display();

	if (++animationPosition > maxPosition)
		animationPosition = 0;

	return DELAY_TIME_BOTH;
}


