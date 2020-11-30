/*
 * StandardCoreAnimation.cpp
 *
 *  Created on: 2 Mar 2018
 *      Author: dancl
 */

#include "StandardCoreAnimation.h"
#include "AudioControl.h"
#include "ApplicationState.h"
#include <math.h>
#include "esp_log.h"

#define TIME_CORE						(1109)
#define TIME_CORE_ITERATION_GAP			(300)
#define TIME_CORE_WARP					(200)
#define TIME_CORE_WARP_ITERATION_GAP	(20)
#define DELAY_MS						(10)

#define STATE_ROWS			0
#define STATE_GAP			1
#define CENTRAL_STATE_IN	0
#define CENTRAL_STATE_OUT	1

#define RED			100
#define GREEN		78
#define BLUE		255

#define CENTRAL_RED		255
#define CENTRAL_GREEN	110
#define CENTRAL_BLUE	110

#define GAP_STEPS						(20)
#define GAP_STEPS_MIDPOINT				(GAP_STEPS / 2)
#define GAP_R_INCREMENT					((255 - CENTRAL_RED) / GAP_STEPS_MIDPOINT)
#define GAP_G_INCREMENT					((255 - CENTRAL_GREEN) / GAP_STEPS_MIDPOINT)
#define GAP_B_INCREMENT					((255 - CENTRAL_BLUE) / GAP_STEPS_MIDPOINT)

StandardCoreAnimation::StandardCoreAnimation(WarpCoreVisual &_core)
	: core(_core)
{
}

StandardCoreAnimation::~StandardCoreAnimation()
{

}

void StandardCoreAnimation::start()
{
	// Clear initial state
	state = STATE_ROWS;
	row = -3;
	centralStep = 0;
	centralState = CENTRAL_STATE_IN;
	frameCount = 0;

	// Here be dragons. Below timing is a bit odd, I don't really understand how it works...

	// Rows timing
	float audioTime = (type == CORE_ANIM_IDLE) ? TIME_CORE : TIME_CORE_WARP;
	frameDivider = floorf(((audioTime / (float)(core.getCentralRowIndex() + 2)) / 2.0f) / (float)DELAY_MS);
	if (frameDivider <= 0)
		frameDivider = 1;

	// Central chamber timing
	float gapTime = (type == CORE_ANIM_IDLE) ? TIME_CORE_ITERATION_GAP : TIME_CORE_WARP_ITERATION_GAP;
	centralFrameDivider = floorf(gapTime / (float)GAP_STEPS_MIDPOINT / (float)DELAY_MS);
	if (centralFrameDivider <= 0)
		centralFrameDivider = 1;
	centralFrameThreshold = ((audioTime / 2.0f) - gapTime) / (float)DELAY_MS;
}

void StandardCoreAnimation::end()
{
	row = -3;
}

uint16_t StandardCoreAnimation::animate()
{
	uint16_t delay = 1;

	switch (state)
	{
	case STATE_ROWS:
		delay = animateRows();
		break;

	case STATE_GAP:
		core.clear();
		delay = animateCentral();
		if (frameCount == 0)
			return delay;
		break;

	default:
		start();
		break;
	}

	++frameCount;

	return delay;
}

uint16_t StandardCoreAnimation::animateRows()
{
	if (frameCount == 0 && soundOn)
	{
		if (type == CORE_ANIM_IDLE)
			playAudioFile(WAV_CORE);
		else
			playAudioFile(WAV_CORE_MAX);
	}

	core.clear();

	if (frameCount % frameDivider == 0)
		++row;

	uint8_t r = RED;
	uint8_t g = GREEN;
	uint8_t b = BLUE;
	uint8_t numRows = core.getNumRows();
	uint8_t centralIndex = core.getCentralRowIndex();

	if (customColourActive)
	{
		r = requestedColour.r;
		g = requestedColour.g;
		b = requestedColour.b;
	}

	for (uint8_t subRow = 0; subRow < 3; ++subRow)
	{
		// Top side
		int16_t topRow = row + subRow;
		if (topRow >= 0 && topRow < centralIndex)
			core.setRow(topRow, r, g, b);

		// Bottom side
		int16_t bottomRow = numRows - (row + subRow) - 1;
		if (bottomRow < numRows && bottomRow > centralIndex)
			core.setRow(bottomRow, r, g, b);
	}

	// Central is always off-white unless overridden
	if (!customColourActive)
		core.setRow(centralIndex, CENTRAL_RED, CENTRAL_GREEN, CENTRAL_BLUE);
	else
		core.setRow(centralIndex, r, g, b);

	if (frameCount >= centralFrameThreshold)
		animateCentral();
	else
		core.display();

	if (row >= centralIndex + 2)
	{
		row = -3;
		state = STATE_GAP;
		return animateCentral();
	}

	return DELAY_MS;
}

uint16_t StandardCoreAnimation::animateCentral()
{
	// Custom colour is solid all the time, so just set to colour and resume animation after time elapses
	if (customColourActive)
	{
		core.setRow(core.getCentralRowIndex(), requestedColour.r, requestedColour.g, requestedColour.b);
		core.display();
		state = STATE_ROWS;
		return (type == CORE_ANIM_IDLE) ? TIME_CORE_ITERATION_GAP : TIME_CORE_WARP_ITERATION_GAP;
	}

	if (frameCount % centralFrameDivider == 0)
	{
		if (centralState == CENTRAL_STATE_IN)
		{
			if (++centralStep > GAP_STEPS_MIDPOINT)
			{
				centralStep = GAP_STEPS_MIDPOINT;
				centralState = CENTRAL_STATE_OUT;
			}
		}
		else if (centralState == CENTRAL_STATE_OUT)
		{
			if (--centralStep <= 0)
			{
				centralStep = 0;
				centralState = CENTRAL_STATE_IN;
				state = STATE_ROWS;
				frameCount = 0;
			}
		}
	}

	uint8_t r = CENTRAL_RED + (centralStep * GAP_R_INCREMENT);
	uint8_t g = CENTRAL_GREEN + (centralStep * GAP_G_INCREMENT);
	uint8_t b = CENTRAL_BLUE + (centralStep * GAP_B_INCREMENT);

	core.setRow(core.getCentralRowIndex(), r, g, b);
	core.display();

	return DELAY_MS;
}
