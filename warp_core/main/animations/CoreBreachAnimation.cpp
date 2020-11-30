/*
 * CoreBreachAnimation.cpp
 *
 *  Created on: 9 Mar 2018
 *      Author: dancl
 */

#include "CoreBreachAnimation.h"
#include "AudioControl.h"
#include "ApplicationState.h"

#define ANIMATION_TIME_MS		(900)
#define LOOP_COUNT_SOUND		(2)

CoreBreachAnimation::CoreBreachAnimation(WarpCoreVisual &_core)
	: core(_core)
{
}

CoreBreachAnimation::~CoreBreachAnimation()
{
}

void CoreBreachAnimation::start()
{
	// Reset state
	animationProgress = 0;
	loopCount = 0;

	delayMs = ANIMATION_TIME_MS / core.getNumCols();
}

void CoreBreachAnimation::end()
{
	animationProgress = 0;
}

uint16_t CoreBreachAnimation::animate()
{
	if (loopCount == 0 && animationProgress == 0 && soundOn)
		playAudioFile(WAV_BREACH);

	uint8_t numCols = core.getNumCols();

	core.clear();
	core.setCol(animationProgress++, 255, 0, 0, true);
	core.display();

	if (animationProgress >= numCols)
	{
		animationProgress = 0;
		if (++loopCount >= LOOP_COUNT_SOUND)
			loopCount = 0;
	}


	return delayMs;
}
