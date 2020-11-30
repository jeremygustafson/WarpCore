/*
 * RedAlertVoyagerAnimation.cpp
 *
 *  Created on: 3 Mar 2018
 *      Author: dancl
 */

#include "RedAlertVoyagerAnimation.h"
#include "Config.h"
#include "AudioControl.h"
#include "ApplicationState.h"

#define STATE_ANIMATE_IN		0
#define STATE_CYCLE_IN			1
#define STATE_CYCLE_OUT			2

#define TIME_RED_ALERT			(1100)
#define CYCLE_DELAY_MS			25
#define DELAY_BETWEEN_ALERTS	(1000)

RedAlertVoyagerAnimation::RedAlertVoyagerAnimation(WarpCoreVisual &_core)
	: core(_core)
{
}

RedAlertVoyagerAnimation::~RedAlertVoyagerAnimation()
{
}

void RedAlertVoyagerAnimation::start()
{
	float audioTime = TIME_RED_ALERT;

	// Reset state
	state = STATE_ANIMATE_IN;
	animationProgress = 0;

	// Pre-calculate increments / multipliers
	delayMs = (audioTime / 2.0f) / ((float)core.getNumRows() / 2.0f);
	redIncrement = 255.0f / ((audioTime / 2.0f) / (float)CYCLE_DELAY_MS);
}

void RedAlertVoyagerAnimation::end()
{
	animationProgress = 0;
}

uint16_t RedAlertVoyagerAnimation::animate()
{
	switch (state)
	{
	case STATE_ANIMATE_IN:
		return animateFadeIn();

	default:
		return animateCycle();
	}
}

uint16_t RedAlertVoyagerAnimation::animateFadeIn()
{
	if (animationProgress == 0 && soundOn)
		playAudioFile(WAV_RED_ALERT_VGR);

	uint8_t centralIndex = core.getCentralRowIndex();

	core.clear();
	core.setRow(centralIndex, 255, 0, 0);
	for (uint8_t displayRow = 0; displayRow < animationProgress; ++displayRow)
	{
		core.setRow(centralIndex - displayRow - 1, 255, 0, 0);
		core.setRow(centralIndex + displayRow + 1, 255, 0, 0);
	}

	if (++animationProgress >= centralIndex)
	{
		animationProgress = 255;
		state = STATE_CYCLE_OUT;
	}

	core.display();
	return delayMs;
}

uint16_t RedAlertVoyagerAnimation::animateCycle()
{
	uint16_t delay = CYCLE_DELAY_MS;

	// Update red value
	if (state == STATE_CYCLE_OUT)
	{
		animationProgress -= redIncrement;
		if (animationProgress <= 0)
		{
			animationProgress = 0;
			state = STATE_CYCLE_IN;
			delay = DELAY_BETWEEN_ALERTS;
		}
	}
	else
	{
		if (animationProgress == 0 && soundOn)
			playAudioFile(WAV_RED_ALERT_VGR);

		animationProgress += redIncrement;
		if (animationProgress >= 255)
		{
			animationProgress = 255;
			state = STATE_CYCLE_OUT;
		}
	}

	core.clear(animationProgress, 0, 0);
	core.display();

	return delay;
}
