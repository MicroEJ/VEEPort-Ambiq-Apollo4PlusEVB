/*
 * C
 *
 * Copyright 2022-2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include "LLUI_INPUT_impl.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include "interrupts.h"

#include "buttons_driver.h"
#include "touch_manager.h"

void LLUI_INPUT_IMPL_initialize(void) {
	BUTTONS_DRIVER_initialize();
	TOUCH_MANAGER_initialize();

}

jint LLUI_INPUT_IMPL_getInitialStateValue(jint stateMachinesID, jint stateID) {
	(void) stateMachinesID;
	(void) stateID;
	return 0;

}

void LLUI_INPUT_IMPL_enterCriticalSection(void) {
	BUTTONS_DRIVER_disable_interrupts();
}

void LLUI_INPUT_IMPL_leaveCriticalSection(void) {
	BUTTONS_DRIVER_enable_interrupts();
}
