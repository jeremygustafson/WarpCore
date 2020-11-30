/*
 * SpiralAnimation.h
 *
 *  Created on: 9 Mar 2018
 *      Author: dancl
 */

#ifndef MAIN_ANIMATIONS_SPIRALANIMATION_H_
#define MAIN_ANIMATIONS_SPIRALANIMATION_H_

#include "WarpCoreAnimator.h"
#include "WarpCoreVisual.h"

class SpiralAnimation : public WarpCoreAnimation
{
public:
	SpiralAnimation(WarpCoreVisual &core);
	virtual ~SpiralAnimation();

	virtual void start();
	virtual void end();
	virtual uint16_t animate();

private:
	WarpCoreVisual &core;

	uint8_t state = 0;
	uint8_t *rowStates = NULL;
	uint8_t numRows = 0;
	uint8_t numCols = 0;
	uint8_t endRow = 0;
	uint8_t colourPosition = 0;
	uint8_t colourLadder = 0;

	void animateIn();
	void animateRotation();
	led_t getPixelColour(uint16_t position);
};

#endif /* MAIN_ANIMATIONS_SPIRALANIMATION_H_ */
