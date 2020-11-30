/*
 * RedAlertAnimation.cpp
 *
 *  Created on: 3 Mar 2018
 *      Author: dancl
 */

#include "RedAlertAnimation.h"
#include "Config.h"
#include "AudioControl.h"
#include "ApplicationState.h"
#include "esp_log.h"

#define STATE_ANIMATE_IN		0
#define STATE_CYCLE_IN			1
#define STATE_CYCLE_OUT			2

#define TIME_RED_ALERT			(900)
#define CYCLE_DELAY_MS			25
#define DELAY_BETWEEN_ALERTS	(900)

RedAlertAnimation::RedAlertAnimation(WarpCoreVisual &_core)
	: core(_core)
{
}

RedAlertAnimation::~RedAlertAnimation()
{
}

void RedAlertAnimation::start()
{
	float audioTime = TIME_RED_ALERT;

	// Reset state
	state = STATE_ANIMATE_IN;
	animationProgress = 0;

	// Pre-calculate increments / multipliers
	delayMs = (audioTime / 2.0f) / ((float)core.getNumRows() / 2.0f);
	redIncrement = 255.0f / ((audioTime / 2.0f) / (float)CYCLE_DELAY_MS);

	ESP_LOGI("RedAlert", "Red increment: %i", redIncrement);
}

void RedAlertAnimation::end()
{
	animationProgress = 0;
}

uint16_t RedAlertAnimation::animate()
{
	switch (state)
	{
	case STATE_ANIMATE_IN:
		return animateFadeIn();

	default:
		return animateCycle();
	}
}

uint16_t RedAlertAnimation::animateFadeIn()
{
	if (animationProgress == 0 && soundOn)
		playAudioFile(WAV_RED_ALERT);

	core.clear();

	uint8_t centralIndex = core.getCentralRowIndex();
	uint8_t numRows = core.getNumRows();
	float centralDelta = animationProgress / numRows;
	uint8_t centralRed = 255.0f * centralDelta;

	core.setRow(centralIndex, centralRed, 0, 0);
	for (uint8_t displayRow = 0; displayRow < animationProgress; ++displayRow)
	{
		uint8_t bottomRow = numRows - displayRow - 1;
		core.setRow(displayRow, 255, 0, 0);
		core.setRow(bottomRow, 255, 0, 0);
	}

	if (++animationProgress >= centralIndex)
	{
		animationProgress = 255;
		state = STATE_CYCLE_OUT;
	}

	core.display();
	return delayMs;
}

uint16_t RedAlertAnimation::animateCycle()
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
			playAudioFile(WAV_RED_ALERT);

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
