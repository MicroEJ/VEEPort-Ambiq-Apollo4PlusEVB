/*
 * C
 *
 * Copyright 2022-2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// this h file is created by buildSystemMicroUI step
#include "microui_constants.h"
#include "event_generator.h"

#include "LLUI_INPUT.h"
#include "microui_event_decoder_conf.h"

int32_t EVENT_GENERATOR_button_pressed(int32_t button_id) {
   (void) button_id;
	return LLUI_INPUT_sendCommandEvent(MICROUI_EVENTGEN_COMMANDS, 0);
}

int32_t EVENT_GENERATOR_button_repeated(int32_t button_id) {
   (void) button_id;
	return LLUI_INPUT_sendCommandEvent(MICROUI_EVENTGEN_COMMANDS, 0);
}

int32_t EVENT_GENERATOR_button_released(int32_t button_id) {
   (void) button_id;
   return LLUI_INPUT_OK; // the event has been managed
}

int32_t EVENT_GENERATOR_touch_pressed(int32_t x, int32_t y)
{
   return LLUI_INPUT_sendTouchPressedEvent(MICROUI_EVENTGEN_TOUCH, x, y);
}

int32_t EVENT_GENERATOR_touch_moved(int32_t x, int32_t y)
{
   return LLUI_INPUT_sendTouchMovedEvent(MICROUI_EVENTGEN_TOUCH, x, y);
}

int32_t EVENT_GENERATOR_touch_released(void)
{
   return LLUI_INPUT_sendTouchReleasedEvent(MICROUI_EVENTGEN_TOUCH);
}
