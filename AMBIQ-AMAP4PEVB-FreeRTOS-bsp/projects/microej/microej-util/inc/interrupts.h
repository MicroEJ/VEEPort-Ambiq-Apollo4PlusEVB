/*
 * C
 *
 * Copyright 2013-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief MicroEJ util
 * @author MicroEJ Developer Team
 * @version 1.3.0
 * @date 3 August 2017
 */

#ifndef _INTERRUPTS
#define _INTERRUPTS

/* Includes ------------------------------------------------------------------*/

#include "microej.h"

/* API -----------------------------------------------------------------------*/

/**
 * This function must be called when entering in an interrupt.
 */
uint8_t interrupt_enter(void);


/**
 * This function must be called when leaving an interrupt.
 */
void interrupt_leave(uint8_t leave);

/**
 * This function returns MICROEJ_TRUE or MICROEJ_FALSE to indicate if an ISR is currently executed.
 */
uint8_t interrupt_is_in(void);

#endif
