/*
 * StackAnimation.h
 *
 *  Created on: 9 Mar 2018
 *      Author: dancl
 */

#ifndef MAIN_ANIMATIONS_STACKANIMATION_H_
#define MAIN_ANIMATIONS_STACKANIMATION_H_

#include "WarpCoreAnimator.h"
#include "WarpCoreVisual.h"

class StackAnimation : public WarpCoreAnimation
{
public:
	StackAnimation(WarpCoreVisual &core);
	virtual ~StackAnimation();

	virtual void start();
	virtual void end();
	virtual uint16_t animate();

private:
	WarpCoreVisual &core;
	uint8_t state = 0;

	led_t colour = {};
	uint8_t currentPosition = 0;
	int8_t destinationRow = 0;

	void stackUp();
	void stackDown();
};

#endif /* MAIN_ANIMATIONS_STACKANIMATION_H_ */
