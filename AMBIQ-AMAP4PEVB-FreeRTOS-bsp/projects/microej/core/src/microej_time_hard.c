/*
 * C
 *
 * Copyright 2018-2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include "microej_time.h"
#include "microej.h"

#include "FreeRTOS.h"

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

/*
 *********************************************************************************************************
 * 	                                      DEFINES
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * 	                                      INTERNAL FUNCTIONS
 *********************************************************************************************************
 */

// this value is updated at each interrupt
static int64_t software_counter ;

/** Offset in ms from system time to application time */
static uint64_t microej_application_time_offset = 0;

/*
 *********************************************************************************************************
 * 	                                      PRIVATE FUNCTIONS
 *********************************************************************************************************
 */

static int64_t timer_get_counter_value(void)
{
	int64_t tc = am_hal_stimer_counter_get();
	return tc & (int64_t)0xffffffffU ;
}

int64_t time_hardware_timer_getTicks(void)
{
	return software_counter + timer_get_counter_value();
}

/*
 * An interrupt can occur between the reading of software_counter
 * and timer_get_counter_value(). So the value of software_counter is not
 * coherent with the value of timer_get_counter_value().
 * Compute twice the ticks value. At least one will be correct.
 */
static int64_t time_hardware_timer_getAndCheckTicks(void)
{
	volatile int64_t val1 = time_hardware_timer_getTicks();
	volatile int64_t val2 = time_hardware_timer_getTicks();
	int64_t ret;

	if(val2 < val1)
	{	// second computation get back in the past: val2 is not valid for sure
		ret = val1;
	}
	else
	{	// first computation may be invalid (to far in the past): val2 is valid for sure
		ret = val2;
	}
	return ret;
}


static int64_t timer_get_max_counter_value(void)
{
	return MAX_STIMER_VALUE ;
}

//*****************************************************************************
//
// Timer Interrupt Service Routine (ISR)
//
//*****************************************************************************
void am_stimerof_isr(void)
{
    NVIC_ClearPendingIRQ(STIMER_OVF_IRQn);
    software_counter += timer_get_max_counter_value() ;

}

/*
 *********************************************************************************************************
 * 	                                      PUBLIC FUNCTIONS
 *********************************************************************************************************
 */

void microej_time_init(void)
{
        NVIC_DisableIRQ(STIMER_OVF_IRQn);
	//
	// Enable overflow interrupt in STIMER
	//
	am_hal_stimer_int_enable(AM_HAL_STIMER_INT_OVERFLOW);
        am_hal_stimer_reset_config();

	//
	// Configure the STIMER and run
	//
	am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
	am_hal_stimer_config(configSTIMER_SRC | AM_HAL_STIMER_CFG_RUN);
        
        //
	// Enable the timer interrupt in the NVIC.
	//
        NVIC_ClearPendingIRQ(STIMER_OVF_IRQn);
	NVIC_SetPriority(STIMER_OVF_IRQn, AM_IRQ_PRIORITY_DEFAULT);
	NVIC_EnableIRQ(STIMER_OVF_IRQn);

	am_util_stdio_printf("STIMER init done!\n\n");
}

int64_t microej_time_get_current_time(uint8_t is_platform_time)
{
	/*
	 * /!\
	 * isPlatformTime == true when ej.bon.Util.platformTimeMillis
	 * isPlatformTime == false when java.lang.System.currentTimeMillis
	 * /!\
	 */
	int64_t systemTime = time_hardware_timer_getTicks() * (int64_t)1000 / configSTIMER_RATE_HZ ;
	if (is_platform_time != MICROEJ_TRUE) {
		systemTime += (int64_t) microej_application_time_offset;
	}
	return systemTime;
}

int64_t microej_time_get_time_nanos(void)
{
	return time_hardware_timer_getTicks()  * (int64_t)1000000000 / configSTIMER_RATE_HZ;
}

void microej_time_set_application_time(int64_t time)
{
	int64_t currentTime = (int64_t)microej_time_get_current_time(MICROEJ_TRUE);
	microej_application_time_offset = time - currentTime;
}

int64_t microej_time_time_to_tick(int64_t time)
{
	int64_t output;
	int64_t ticks64;
	uint8_t overflows = MICROEJ_TRUE;

	if(0 >= time){
		overflows = MICROEJ_FALSE;
		output = 0;
	}else{
		int64_t mticks = time * (int64_t)configTICK_RATE_HZ; // milli-ticks
		// Check for no overflow
		if ( (mticks >= 0) ) {
			ticks64 = (mticks + 999LL) / 1000LL;
			// Check for no overflow
			if ( (ticks64 >= 0) ) {
				TickType_t ticks = (TickType_t)ticks64;
				// Check for no overflow
				if ( (ticks == (uint32_t)ticks64) ) {
					output = (int64_t)ticks;
					overflows = MICROEJ_FALSE;
				}
			}
		}
	}

	if ((uint8_t) MICROEJ_TRUE == overflows ) {
		output = (int64_t)UINT32_MAX;
	}

	return output;
}


