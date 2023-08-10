/*
 * C
 *
 * Copyright 2015-2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */


/* Includes ------------------------------------------------------------------*/

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "touch_helper.h"
#include "touch_manager.h"
#include "microej.h"
#include "am_bsp.h"
#include "nema_event.h"
#include "am_util_stdio.h"
#include "ui_buffer.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief Task delay (in ms) between detecting a multiple pressed events
 */
#define TOUCH_DELAY 10

/*
 * @brief Task stack size
 */
#define TOUCH_STACK_SIZE  (128)
#define TOUCH_TASK_PRIORITY ( 9 )

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------

SemaphoreHandle_t touch_interrupt_sem;

/* Private API ---------------------------------------------------------------*/

/**
 * Touch thread routine
 */
static void __touch_manager_task(void *pvParameters);

/* API -----------------------------------------------------------------------*/

void TOUCH_MANAGER_initialize(void)
{

    touch_interrupt_sem = xSemaphoreCreateBinary();

  	//nema_event_init(1, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

	(void)xTaskCreate(__touch_manager_task, "Touch screen task", TOUCH_STACK_SIZE, NULL, TOUCH_TASK_PRIORITY, NULL);


}

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------


// See the section 'Internal function definitions' for the function documentation
static void __touch_manager_read(void)
{
	if (!touch_release)
	{
		TOUCH_HELPER_pressed(g_s_buf_touch.mouse_x, g_s_buf_touch.mouse_y);
	}
		else
	{
		// Send Release event
		TOUCH_HELPER_released();

	}
}

// See the section 'Internal function definitions' for the function documentation
static void __touch_manager_task(void *pvParameters)
{
	(void) pvParameters;
	while (1)
	{

		/* Suspend ourselves */
		xSemaphoreTake(touch_interrupt_sem, portMAX_DELAY);

		/* We have been woken up, lets work ! */
		__touch_manager_read();
                vTaskDelay(TOUCH_DELAY);
	}
}