/*
 * KnightRiderAnimation.h
 *
 *  Created on: 17 Mar 2018
 *      Author: dancl
 */

#ifndef MAIN_ANIMATIONS_KNIGHTRIDERANIMATION_H_
#define MAIN_ANIMATIONS_KNIGHTRIDERANIMATION_H_

#include "WarpCoreAnimator.h"
#include "WarpCoreVisual.h"

class KnightRiderAnimation : public WarpCoreAnimation
{
public:
	KnightRiderAnimation(WarpCoreVisual &core);
	virtual ~KnightRiderAnimation();

	virtual void start();
	virtual void end();
	virtual uint16_t animate();

private:
	WarpCoreVisual &core;
	int16_t dimSubtract;
	uint8_t numRows = 0;
	int16_t min = 0;
	int16_t max = 0;
	int16_t range = 0;

	led_t *rowColours = NULL;
	uint8_t state = 0;
	int16_t currentPosition = 0;

	void dimTrail();
	led_t getColour();
};

#endif /* MAIN_ANIMATIONS_KNIGHTRIDERANIMATION_H_ */
