/*
 * CoreBreachAnimation.h
 *
 *  Created on: 9 Mar 2018
 *      Author: dancl
 */

#ifndef MAIN_ANIMATIONS_COREBREACHANIMATION_H_
#define MAIN_ANIMATIONS_COREBREACHANIMATION_H_

#include "WarpCoreAnimator.h"
#include "WarpCoreVisual.h"

class CoreBreachAnimation : public WarpCoreAnimation
{
public:
	CoreBreachAnimation(WarpCoreVisual &core);
	virtual ~CoreBreachAnimation();

	virtual void start();
	virtual void end();
	virtual uint16_t animate();

private:
	WarpCoreVisual &core;
	uint16_t delayMs = 250;
	uint8_t animationProgress = 0;
	uint8_t loopCount = 0;
};

#endif /* MAIN_ANIMATIONS_COREBREACHANIMATION_H_ */
