/*
 * RainbowAnimation.h
 *
 *  Created on: 3 Mar 2018
 *      Author: dancl
 *
 *  Animation based on Adafruit Strand Test Example: https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/strandtest/strandtest.ino
 */

#ifndef MAIN_ANIMATIONS_RAINBOWANIMATION_H_
#define MAIN_ANIMATIONS_RAINBOWANIMATION_H_

#include "WarpCoreAnimator.h"
#include "WarpCoreVisual.h"

#define RAINBOW_FULL		(0)
#define RAINBOW_CHASE_ROW	(1)
#define RAINBOW_CHASE_COL	(2)
#define RAINBOW_CHASE_BOTH	(3)

class RainbowAnimation : public WarpCoreAnimation
{
public:
	RainbowAnimation(WarpCoreVisual &core);
	virtual ~RainbowAnimation();

	virtual void start();
	virtual void end();
	virtual uint16_t animate();

	void setType(uint8_t rainbowType) { type = rainbowType; }
private:
	WarpCoreVisual &core;
	volatile uint8_t type = RAINBOW_FULL;
	uint16_t animationPosition = 0;

	uint16_t animateFull();
	uint16_t animateRow();
	uint16_t animateCol();
	uint16_t animateBoth();
};

#endif /* MAIN_ANIMATIONS_RAINBOWANIMATION_H_ */
