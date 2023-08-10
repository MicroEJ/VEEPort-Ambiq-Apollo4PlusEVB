/*
 * C
 *
 * Copyright 2015-2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */


#ifndef TOUCH_MANAGER
#define TOUCH_MANAGER

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

#if defined(AM_PART_APOLLO4B) 
#include "apollo4b.h"
#elif defined(AM_PART_APOLLO4P)
#include "apollo4p.h"
#else
#error "Need to define the board in preporcessor (AM_PART_APOLLO4B or AM_PART_APOLLO4P)"
#endif

#include "nema_event.h"


extern bool i2c_read_flag;
extern bool touch_release;

extern nema_event_t g_s_last_event;
extern nema_event_t g_s_buf_touch;

extern SemaphoreHandle_t touch_interrupt_sem;

/* API -----------------------------------------------------------------------*/

void TOUCH_MANAGER_initialize(void);
void TOUCH_MANAGER_interrupt(void);

#endif
