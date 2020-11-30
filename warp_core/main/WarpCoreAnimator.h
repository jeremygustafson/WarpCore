/*
 * WarpCoreAnimator.h
 *
 *  Created on: 2 Mar 2018
 *      Author: dancl
 */

#ifndef MAIN_WARPCOREANIMATOR_H_
#define MAIN_WARPCOREANIMATOR_H_

#include "TaskCPP.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"

class WarpCoreAnimation
{
public:
	virtual void start() = 0;
	virtual void end() = 0;
	virtual uint16_t animate() = 0;
};

class WarpCoreAnimator : public TaskClass
{
public:
	WarpCoreAnimator();
	virtual ~WarpCoreAnimator();

	void setAnimation(WarpCoreAnimation *animation);

protected:
	virtual void task();

private:
	SemaphoreHandle_t animationSemaphore;
	WarpCoreAnimation *currentAnimation = NULL;
};

#endif /* MAIN_WARPCOREANIMATOR_H_ */
