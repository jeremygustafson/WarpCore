/*
 * WarpCoreAnimator.cpp
 *
 *  Created on: 2 Mar 2018
 *      Author: dancl
 */

#include "WarpCoreAnimator.h"

const char *ANIM_TAG = "Animator";

WarpCoreAnimator::WarpCoreAnimator()
	: TaskClass("animator", 9, 4096, 0)
{
	animationSemaphore = xSemaphoreCreateMutex();
	assert(animationSemaphore != NULL);
}

WarpCoreAnimator::~WarpCoreAnimator()
{
}

void WarpCoreAnimator::task()
{
	while (true)
	{
		uint16_t sleepTime = 1000; // Default to 1 second sleep if no animations set up

		if (xSemaphoreTake(animationSemaphore, portMAX_DELAY) == pdFALSE)
			continue;

		if (currentAnimation != NULL)
			sleepTime = currentAnimation->animate();

		xSemaphoreGive(animationSemaphore);
		vTaskDelay(sleepTime / portTICK_PERIOD_MS);
	}
}

void WarpCoreAnimator::setAnimation(WarpCoreAnimation *animation)
{
	if (xSemaphoreTake(animationSemaphore, portMAX_DELAY) == pdFALSE)
		return;

	if (currentAnimation != NULL)
		currentAnimation->end();

	currentAnimation = animation;
	animation->start();

	xSemaphoreGive(animationSemaphore);
}
