/*
 * C
 *
 * Copyright 2021-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#ifndef _LLMJVM_FREERTOS_CONFIGURATION_H
#define _LLMJVM_FREERTOS_CONFIGURATION_H

/* Defines used to include FreeRTOS API header files. Update it if header file location is different. */

#define FREERTOS_HEADER				"FreeRTOS.h"
#define FREERTOS_TIMERS_HEADER		"timers.h"
#define FREERTOS_SEMPHR_HEADER		"semphr.h"
#define FREERTOS_TASK_HEADER		"task.h"
#define YIELD_FROM_ISR(x)			portYIELD_FROM_ISR(x)
#define IS_INSIDE_INTERRUPT			xPortIsInsideInterrupt

#endif /* _LLMJVM_FREERTOS_CONFIGURATION_H */
