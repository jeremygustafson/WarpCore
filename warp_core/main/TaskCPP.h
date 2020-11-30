// From: https://www.freertos.org/FreeRTOS_Support_Forum_Archive/July_2010/freertos_Is_it_possible_create_freertos_task_in_c_3778071.html

#ifndef TaskCPP_H
#define TaskCPP_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class TaskBase
{
public:
	xTaskHandle handle;

	virtual ~TaskBase()
	{
#if INCLUDE_vTaskDelete
		vTaskDelete(handle);
#endif
		return;
	}
};

class Task: public TaskBase
{
public:
	Task(char const*name, void (*taskfun)(void *), unsigned int priority,
			unsigned int stackDepth = 1024, int8_t coreNum = -1)
	{
		if (coreNum == -1)
			xTaskCreate(taskfun, name, stackDepth, this, priority, &handle);
		else
			xTaskCreatePinnedToCore(taskfun, name, stackDepth, this, priority, &handle, (BaseType_t)coreNum);
	}

};

class TaskClass: public TaskBase
{
public:
	TaskClass(char const*name, unsigned int priority, unsigned int stackDepth = 1024, int8_t coreNum = -1)
	{
		if (coreNum == -1)
			xTaskCreate(&taskfun, name, stackDepth, this, priority, &handle);
		else
			xTaskCreatePinnedToCore(&taskfun, name, stackDepth, this, priority, &handle, (BaseType_t)coreNum);
	}

protected:
	virtual void task() = 0;

	static void taskfun(void* parm)
	{
		((TaskClass*)parm)->task();
		//static_cast<TaskClass*>(parm)->task();

		while (true)
			vTaskDelay(10000);
	}
};

#endif
