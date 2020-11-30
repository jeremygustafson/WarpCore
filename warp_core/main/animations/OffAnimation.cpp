/*
 * OffAnimation.cpp
 *
 *  Created on: 21 Mar 2018
 *      Author: dancl
 */

#include "OffAnimation.h"

OffAnimation::OffAnimation(WarpCoreVisual &_core)
	: core(_core)
{
}

OffAnimation::~OffAnimation()
{
}

void OffAnimation::start()
{
	core.clear();
	core.display();
}

void OffAnimation::end()
{
}

uint16_t OffAnimation::animate()
{
	return 1000;
}
