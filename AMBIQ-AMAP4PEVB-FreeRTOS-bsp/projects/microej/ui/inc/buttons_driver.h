/*
 * C
 *
 * Copyright 2022-2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#ifndef BUTTONS_DRIVER_H
#define BUTTONS_DRIVER_H


#ifdef __cplusplus
extern "C"
{
#endif


// cppcheck-suppress [misra-c2012-8.2] False positive error
void BUTTONS_DRIVER_initialize(void);
// cppcheck-suppress [misra-c2012-8.2] False positive error
void BUTTONS_DRIVER_disable_interrupts(void);
// cppcheck-suppress [misra-c2012-8.2] False positive error
void BUTTONS_DRIVER_enable_interrupts(void);


#ifdef __cplusplus
}
#endif

#endif
