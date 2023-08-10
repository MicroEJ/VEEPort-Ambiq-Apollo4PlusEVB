/*
 * C
 *
 * Copyright 2022-2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#ifndef EVENT_GENERATOR_H
#define EVENT_GENERATOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif


int32_t EVENT_GENERATOR_button_pressed(int32_t button_id);
int32_t EVENT_GENERATOR_button_repeated(int32_t button_id);
int32_t EVENT_GENERATOR_button_released(int32_t button_id);

int32_t EVENT_GENERATOR_touch_pressed(int32_t x, int32_t y);
int32_t EVENT_GENERATOR_touch_moved(int32_t x, int32_t y);
int32_t EVENT_GENERATOR_touch_released(void);

#ifdef __cplusplus
}
#endif


#endif
