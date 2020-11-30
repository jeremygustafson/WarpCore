/*
 * OffAnimation.h
 *
 *  Created on: 21 Mar 2018
 *      Author: dancl
 */

#ifndef MAIN_ANIMATIONS_OFFANIMATION_H_
#define MAIN_ANIMATIONS_OFFANIMATION_H_

#include "WarpCoreAnimator.h"
#include "WarpCoreVisual.h"

class OffAnimation : public WarpCoreAnimation
{
public:
	OffAnimation(WarpCoreVisual &core);
	virtual ~OffAnimation();

	virtual void start();
	virtual void end();
	virtual uint16_t animate();

private:
	WarpCoreVisual &core;
};

#endif /* MAIN_ANIMATIONS_OFFANIMATION_H_ */
