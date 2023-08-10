/*
 * C
 *
 * Copyright 2021-2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "FreeRTOS.h"
#include "task.h"

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "microej.h"

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

void vApplicationStackOverflowHook( TaskHandle_t pxTask, signed char *pcTaskName ) {
	//
	// Run time stack overflow checking is performed if
	// configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	// function is called if a stack overflow is detected.
	//

	(void) pxTask;
	am_util_stdio_printf("Stack overflow in task: %s\n", pcTaskName);

    do { } while(1);
}

void vApplicationMallocFailedHook(void)
{
  //
  // Called if a call to pvPortMalloc() fails because there is insufficient
  // free memory available in the FreeRTOS heap.  pvPortMalloc() is called
  // internally by FreeRTOS API functions that create tasks, queues, software
  // timers, and semaphores.  The size of the FreeRTOS heap is set by the
  // configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
  //

  am_util_stdio_printf("Malloc failed in pvPortMalloc\n");

  do { } while(1);
}

// This function is not available in current sdk port.c file
BaseType_t xPortIsInsideInterrupt( void )
{
uint32_t ulCurrentInterrupt;
BaseType_t xReturn;

    /* Obtain the number of the currently executing interrupt. */
    __asm volatile( "mrs %0, ipsr" : "=r"( ulCurrentInterrupt ) );

    if( ulCurrentInterrupt == (uint32_t)0 )
    {
        xReturn = pdFALSE;
    }
    else
    {
        xReturn = pdTRUE;
    }

    return xReturn;
}

#define configSYSTICK_CLOCK_HZ configCPU_CLOCK_HZ
#define portMAX_32_BIT_NUMBER  ( 0xffffffffUL )
#define portMISSED_COUNTS_FACTOR			( 45UL )

static uint32_t xMaximumPossibleSuppressedTicks = 0;
static uint32_t ulTimerCountsForOneTick = 0;
//static uint32_t ulStoppedTimerCompensation = 0;

// cppcheck-suppress [misra-c2012-20.9] configUSE_TICKLESS_IDLE defined in the FreeRTOSConfig.h
#if( configUSE_TICKLESS_IDLE == 1 )

void am_stimer_cmpr0_isr(void)
{
    // Clear the timer interrupt status.
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);

    // Disable interrupt
    am_hal_stimer_int_disable(AM_HAL_STIMER_INT_COMPAREA);
}

static void scheduleStimerCompareA(uint32_t delta)
{

	 am_hal_stimer_compare_delta_set(STIMER_STMINTSTAT_COMPAREA_Pos, delta);
	 am_hal_stimer_config(STIMER->STCFG |
	                          AM_HAL_STIMER_CFG_COMPARE_A_ENABLE);

	 am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREA);

	 NVIC_SetPriority(STIMER_CMPR0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
	 NVIC_EnableIRQ(STIMER_CMPR0_IRQn);
}

void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
	uint32_t ulReloadValue;
	//uint32_t ulCompleteTickPeriods, ulCompletedSysTickDecrements;
	TickType_t xModifiableIdleTime;
	/* Make sure the SysTick reload value does not overflow the counter. */
	if( xExpectedIdleTime > xMaximumPossibleSuppressedTicks )
	{
		xExpectedIdleTime = xMaximumPossibleSuppressedTicks;
	}

	/* Stop the SysTick momentarily.  The time the SysTick is stopped for
	is accounted for as best it can be, but using the tickless mode will
	inevitably result in some tiny drift of the time maintained by the
	kernel with respect to calendar time. */
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

	/* Calculate the reload value required to wait xExpectedIdleTime
	tick periods.  -1 is used because this code will execute part way
	through one of the tick periods. */
	ulReloadValue = (xExpectedIdleTime - 1UL) * ( configSTIMER_RATE_HZ/ configTICK_RATE_HZ);
	uint32_t initialStimerCount = am_hal_stimer_counter_get();
	//if( ulReloadValue > ulStoppedTimerCompensation )
	//{
	//	ulReloadValue -= ulStoppedTimerCompensation;
	//}

	/* Enter a critical section but don't use the taskENTER_CRITICAL()
	method as that will mask interrupts that should exit sleep mode. */
	__disable_interrupt();
	__DSB();
	__ISB();

	/* If a context switch is pending or a task is waiting for the scheduler
	to be unsuspended then abandon the low power entry. */
	if( eTaskConfirmSleepModeStatus() == eAbortSleep )
	{
		/* Restart from whatever is left in the count register to complete
		this tick period. */
		SysTick->LOAD = SysTick->VAL;

		/* Restart SysTick. */
		SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

		/* Reset the reload register to the value required for normal tick
		periods. */
		SysTick->LOAD = ulTimerCountsForOneTick - 1UL;

		/* Re-enable interrupts - see comments above __disable_interrupt()
		call above. */
		__enable_interrupt();
	}
	else
	{
		/* Set the STIMER COMPAREA value. */

		//am_util_stdio_printf("Sleep at 0x%X for %d ticks\n", initialStimerCount,  xExpectedIdleTime);
		scheduleStimerCompareA(ulReloadValue);

		/* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
		set its parameter to 0 to indicate that its implementation contains
		its own wait for interrupt or wait for event instruction, and so wfi
		should not be executed again.  However, the original expected idle
		time variable must remain unmodified, so a copy is taken. */
		xModifiableIdleTime = xExpectedIdleTime;
		configPRE_SLEEP_PROCESSING( xModifiableIdleTime );
		if( xModifiableIdleTime > 0 )
		{
			__DSB();
			__WFI();
			__ISB();
		}
		configPOST_SLEEP_PROCESSING( xExpectedIdleTime );

		/* Re-enable interrupts to allow the interrupt that brought the MCU
		out of sleep mode to execute immediately.  see comments above
		__disable_interrupt() call above. */
		__enable_interrupt();
		__DSB();
		__ISB();

		/* Disable interrupts again because the clock is about to be stopped
		and interrupts that execute while the clock is stopped will increase
		any slippage between the time maintained by the RTOS and calendar
		time. */
		__disable_interrupt();
		__DSB();
		__ISB();

		/* Disable the SysTick clock without reading the
		portNVIC_SYSTICK_CTRL_REG register to ensure the
		portNVIC_SYSTICK_COUNT_FLAG_BIT is not cleared if it is set.  Again,
		the time the SysTick is stopped for is accounted for as best it can
		be, but using the tickless mode will inevitably result in some tiny
		drift of the time maintained by the kernel with respect to calendar
		time*/
		// portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT | portNVIC_SYSTICK_INT_BIT );

		/* Determine if the SysTick clock has already counted to zero and
		been set back to the current reload value (the reload back being
		correct for the entire expected idle time) or if the SysTick is yet
		to count to zero (in which case an interrupt other than the SysTick
		must have brought the system out of sleep mode). */
	//	if( ( portNVIC_SYSTICK_CTRL_REG & portNVIC_SYSTICK_COUNT_FLAG_BIT ) != 0 )
	//	{
	//		uint32_t ulCalculatedLoadValue;
    //
			/* The tick interrupt is already pending, and the SysTick count
			reloaded with ulReloadValue.  Reset the
			portNVIC_SYSTICK_LOAD_REG with whatever remains of this tick
			period. */
	//		ulCalculatedLoadValue = ( ulTimerCountsForOneTick - 1UL ) - ( ulReloadValue - portNVIC_SYSTICK_CURRENT_VALUE_REG );
    //
			/* Don't allow a tiny value, or values that have somehow
			underflowed because the post sleep hook did something
			that took too long. */
	//		if( ( ulCalculatedLoadValue < ulStoppedTimerCompensation ) || ( ulCalculatedLoadValue > ulTimerCountsForOneTick ) )
	//		{
	//			ulCalculatedLoadValue = ( ulTimerCountsForOneTick - 1UL );
	//		}
    //
	//		portNVIC_SYSTICK_LOAD_REG = ulCalculatedLoadValue;
    //
			/* As the pending tick will be processed as soon as this
			function exits, the tick value maintained by the tick is stepped
			forward by one less than the time spent waiting. */
	//		ulCompleteTickPeriods = xExpectedIdleTime - 1UL;
	//	}
	//	else
	//	{
			/* Something other than the tick interrupt ended the sleep.
			Work out how long the sleep lasted rounded to complete tick
			periods (not the ulReload value which accounted for part
			ticks). */
	//		ulCompletedSysTickDecrements = ( xExpectedIdleTime * ulTimerCountsForOneTick ) - portNVIC_SYSTICK_CURRENT_VALUE_REG;
    //
			/* How many complete tick periods passed while the processor
			was waiting? */
	//		ulCompleteTickPeriods = ulCompletedSysTickDecrements / ulTimerCountsForOneTick;
    //
			/* The reload value is set to whatever fraction of a single tick
			period remains. */
	//		portNVIC_SYSTICK_LOAD_REG = ( ( ulCompleteTickPeriods + 1UL ) * ulTimerCountsForOneTick ) - ulCompletedSysTickDecrements;
	//	}

		/* Restart SysTick so it runs from portNVIC_SYSTICK_LOAD_REG
		again, then set portNVIC_SYSTICK_LOAD_REG back to its standard
		value. */

		/* disable STIMER COMPAREA interrupt */
		am_hal_stimer_int_disable(AM_HAL_STIMER_INT_COMPAREA);
		 /* Update tick counter with time spent in sleep*/

		uint32_t stepTick = (((am_hal_stimer_counter_get() - initialStimerCount) *configTICK_RATE_HZ/configSTIMER_RATE_HZ));
		//am_util_stdio_printf("Wake at 0x%X skipping %d ticks, 0x%X \n", am_hal_stimer_counter_get(), stepTick, am_hal_stimer_compare_get(0));

		if(stepTick > xExpectedIdleTime)
		{
			stepTick = xExpectedIdleTime;
		}
		vTaskStepTick(stepTick);

		// Restart SYSTICK
		SysTick->VAL = 0;
		SysTick->LOAD = (configCPU_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;
		SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

		/* Exit with interrpts enabled. */
		__enable_interrupt();
	}
}

// cppcheck-suppress [misra-c2012-20.14] False positive, #if above
#endif /* configUSE_TICKLESS_IDLE */
/*-----------------------------------------------------------*/

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
void vPortSetupTimerInterrupt( void )
{
	/* Calculate the constants required to configure the tick interrupt. */
	// cppcheck-suppress [misra-c2012-20.9] configUSE_TICKLESS_IDLE defined in the FreeRTOSConfig.h
	#if( configUSE_TICKLESS_IDLE == 1 )
	{
		ulTimerCountsForOneTick = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ );
		xMaximumPossibleSuppressedTicks = portMAX_32_BIT_NUMBER * configTICK_RATE_HZ/configSTIMER_RATE_HZ;
		//ulStoppedTimerCompensation = portMISSED_COUNTS_FACTOR / ( configCPU_CLOCK_HZ / configSYSTICK_CLOCK_HZ );
	}
	// cppcheck-suppress [misra-c2012-20.14] False positive, #if above
	#endif /* configUSE_TICKLESS_IDLE */

	/* Stop and clear the SysTick. */
	SysTick->CTRL = 0UL;
	SysTick->VAL = 0UL;

	/* Configure SysTick to interrupt at the requested rate. */
	SysTick->LOAD = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
	SysTick->CTRL = ( SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk );
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

