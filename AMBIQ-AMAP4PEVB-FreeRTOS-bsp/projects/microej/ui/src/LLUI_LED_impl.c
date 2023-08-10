/*
 * C
 *
 * Copyright 2022-2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include "LLUI_LED_impl.h"

#include "am_bsp.h"

jint LLUI_LED_IMPL_initialize() {
	am_devices_led_array_init(am_bsp_psLEDs, AM_BSP_NUM_LEDS);
	return AM_BSP_NUM_LEDS;
}

jint LLUI_LED_IMPL_getIntensity(jint led_id) {
	// Contrary to what Ambiq's source file states, 0 means the LED is on, 1 means it is off.
	return (0 == am_devices_led_get(am_bsp_psLEDs, led_id)) ? 1 : 0;
}

void LLUI_LED_IMPL_setIntensity(jint led_id, jint intensity) {
	if (0 != intensity) {
		am_devices_led_on(am_bsp_psLEDs, led_id);
	}
	else {
		am_devices_led_off(am_bsp_psLEDs, led_id);
	}
}

void Java_com_microej_Demo_showLED(void)
{
  jint a = LLUI_LED_IMPL_initialize();
}