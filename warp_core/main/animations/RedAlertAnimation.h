/*
 * RedAlertAnimation.h
 *
 *  Created on: 3 Mar 2018
 *      Author: dancl
 */

#ifndef MAIN_ANIMATIONS_REDALERTANIMATION_H_
#define MAIN_ANIMATIONS_REDALERTANIMATION_H_

#include "WarpCoreAnimator.h"
#include "WarpCoreVisual.h"

class RedAlertAnimation : public WarpCoreAnimation
{
public:
	RedAlertAnimation(WarpCoreVisual &core);
	virtual ~RedAlertAnimation();

	virtual void start();
	virtual void end();
	virtual uint16_t animate();

private:
	WarpCoreVisual &core;
	uint8_t state = 0;
	int16_t animationProgress = 0;
	uint16_t delayMs = 250;
	uint8_t redIncrement = 127;

	uint16_t animateFadeIn();
	uint16_t animateCycle();
};

#endif /* MAIN_ANIMATIONS_REDALERTANIMATION_H_ */
