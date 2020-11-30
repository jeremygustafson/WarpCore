/*
 * SolidColourAnimation.h
 *
 *  Created on: 17 Mar 2018
 *      Author: dancl
 */

#ifndef MAIN_ANIMATIONS_SOLIDCOLOURANIMATION_H_
#define MAIN_ANIMATIONS_SOLIDCOLOURANIMATION_H_

#include "WarpCoreAnimator.h"
#include "WarpCoreVisual.h"

class SolidColourAnimation : public WarpCoreAnimation
{
public:
	SolidColourAnimation(WarpCoreVisual &core);
	virtual ~SolidColourAnimation();

	virtual void start();
	virtual void end();
	virtual uint16_t animate();

private:
	WarpCoreVisual &core;
	led_t colour;
	led_t lastColour = {};
};

#endif /* MAIN_ANIMATIONS_SOLIDCOLOURANIMATION_H_ */
