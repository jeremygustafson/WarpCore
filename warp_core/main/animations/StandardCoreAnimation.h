/*
 * StandardCoreAnimation.h
 *
 *  Created on: 2 Mar 2018
 *      Author: dancl
 */

#ifndef MAIN_STANDARDCOREANIMATION_H_
#define MAIN_STANDARDCOREANIMATION_H_

#include "WarpCoreAnimator.h"
#include "WarpCoreVisual.h"

#define CORE_ANIM_IDLE		(0)
#define CORE_ANIM_WARP		(1)

class StandardCoreAnimation : public WarpCoreAnimation
{
public:
	StandardCoreAnimation(WarpCoreVisual &core);
	virtual ~StandardCoreAnimation();

	virtual void start();
	virtual void end();
	virtual uint16_t animate();

	void setType(uint8_t animType) { type = animType; start(); }

private:
	WarpCoreVisual &core;

	uint8_t type = 0;
	int16_t row = 0;
	uint8_t state = 0;
	uint8_t centralState = 0;
	uint8_t centralStep = 0;
	uint16_t centralFrameThreshold = 0;
	uint16_t centralFrameDivider = 0;
	uint16_t frameCount = 0;
	uint16_t frameDivider = 0;

	uint16_t animateRows();
	uint16_t animateCentral();
};

#endif /* MAIN_STANDARDCOREANIMATION_H_ */
