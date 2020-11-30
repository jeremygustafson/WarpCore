/*
 * RedAlertVoyagerAnimation.h
 *
 *  Created on: 3 Mar 2018
 *      Author: dancl
 */

#ifndef MAIN_ANIMATIONS_REDALERTVOYAGERANIMATION_H_
#define MAIN_ANIMATIONS_REDALERTVOYAGERANIMATION_H_

#include "WarpCoreAnimator.h"
#include "WarpCoreVisual.h"

class RedAlertVoyagerAnimation : public WarpCoreAnimation
{
public:
	RedAlertVoyagerAnimation(WarpCoreVisual &core);
	virtual ~RedAlertVoyagerAnimation();

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

#endif /* MAIN_ANIMATIONS_REDALERTVOYAGERANIMATION_H_ */
