/*
 * C
 *
 * Copyright 2022-2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include "am_bsp.h"

#include "x_ram_checks.h"
#include "x_core_benchmark.h"

#include "am_util.h"
#include "microej_time.h"
#include "FreeRTOSConfig.h"

int printf(const char *format, ...) {
	va_list args;
	va_start(args, format);
	uint32_t result = am_util_stdio_vprintf(format, args);
	va_end(args);
	return result;
}


#define RAM_TEST_AREA_SIZE 50000
uint8_t ram_test_area[RAM_TEST_AREA_SIZE];
uint8_t ram_source_area[RAM_TEST_AREA_SIZE];

X_RAM_CHECKS_zone_t test_zones_32[] = {
	{
		.start_address = (uintptr_t) ram_test_area,
		.end_address = (uintptr_t) (ram_test_area + RAM_TEST_AREA_SIZE),
		0, 0, 0
	}
};

X_RAM_CHECKS_zone_t test_zones_16[] = {
	{
		.start_address = (uintptr_t) ram_test_area,
		.end_address = (uintptr_t) (ram_test_area + RAM_TEST_AREA_SIZE),
		0, 0, 0
	}
};

X_RAM_CHECKS_zone_t test_zones_8[] = {
	{
		.start_address = (uintptr_t) ram_test_area,
		.end_address = (uintptr_t) (ram_test_area + RAM_TEST_AREA_SIZE),
		0, 0, 0
	}
};

X_RAM_CHECKS_zone_t source_zones_32[] = {
	{
		.start_address = (uintptr_t) ram_source_area,
		.end_address = (uintptr_t) (ram_source_area + RAM_TEST_AREA_SIZE),
		0, 0, 0
	}
};

X_RAM_CHECKS_zone_t source_zones_16[] = {
	{
		.start_address = (uintptr_t) ram_source_area,
		.end_address = (uintptr_t) (ram_source_area + RAM_TEST_AREA_SIZE),
		0, 0, 0
	}
};

X_RAM_CHECKS_zone_t source_zones_8[] = {
	{
		.start_address = (uintptr_t) ram_source_area,
		.end_address = (uintptr_t) (ram_source_area + RAM_TEST_AREA_SIZE),
		0, 0, 0
	}
};

X_RAM_CHECKS_zone_t* X_RAM_CHECKS_get32bitZones(void) {
	return test_zones_32;
}

X_RAM_CHECKS_zone_t* X_RAM_CHECKS_get16bitZones(void) {
	return test_zones_16;
}

X_RAM_CHECKS_zone_t* X_RAM_CHECKS_get8bitZones(void) {
	return test_zones_8;
}

X_RAM_CHECKS_zone_t* X_RAM_CHECKS_get32bitSourceZone(void) {
	return source_zones_32;
}

X_RAM_CHECKS_zone_t* X_RAM_CHECKS_get16bitSourceZone(void) {
	return source_zones_16;
}

X_RAM_CHECKS_zone_t* X_RAM_CHECKS_get8bitSourceZone(void) {
	return source_zones_8;
}

uint8_t X_RAM_CHECKS_get32bitZoneNumber(void) {
	return 1;
}

uint8_t X_RAM_CHECKS_get16bitZoneNumber(void) {
	return 1;
}

uint8_t X_RAM_CHECKS_get8bitZoneNumber(void) {
	return 1;
}

bool X_CORE_BENCHMARK_run() {
	core_main();
	return 1;
}

void UTIL_TIME_BASE_initialize(void) {
        microej_time_init();
}

int64_t UTIL_TIME_BASE_getTime(void) {
	return  time_hardware_timer_getTicks() * (int64_t)1000000 / configSTIMER_RATE_HZ ;
}