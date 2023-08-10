/*
 * C
 *
 * Copyright 2022-2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include "microej.h"

#include "am_bsp.h"
#include "am_util.h"
#include "event_generator.h"
#include "buttons_driver.h"

#define BUTTON0_MASK (1<<AM_BSP_GPIO_BUTTON0)


static uint32_t ui32BUTTON0GpioNum = AM_BSP_GPIO_BUTTON0;

static void am_gpio0_001f_isr(void){
    uint32_t ui32IntStatus;
    //
    // Delay for button debounce.
    //
    am_hal_delay_us(20000);

    //
    // Clear the GPIO Interrupt (write to clear).
    //
    AM_CRITICAL_BEGIN
    am_hal_gpio_interrupt_irq_status_get(GPIO0_001F_IRQn, true, &ui32IntStatus);
    
    if (ui32IntStatus == (uint32_t) BUTTON0_MASK)
    {
      if ( g_AM_BSP_GPIO_BUTTON0.GP.cfg_b.eIntDir == AM_HAL_GPIO_PIN_INTDIR_HI2LO)
      {
        g_AM_BSP_GPIO_BUTTON0.GP.cfg_b.eIntDir = AM_HAL_GPIO_PIN_INTDIR_LO2HI;
        (void) EVENT_GENERATOR_button_pressed(0);
      } else if (g_AM_BSP_GPIO_BUTTON0.GP.cfg_b.eIntDir == AM_HAL_GPIO_PIN_INTDIR_LO2HI)
      {
        g_AM_BSP_GPIO_BUTTON0.GP.cfg_b.eIntDir = AM_HAL_GPIO_PIN_INTDIR_HI2LO;
        (void) EVENT_GENERATOR_button_released(0);
      } else {
        // NA
      }
      
      am_hal_gpio_pinconfig(AM_BSP_GPIO_BUTTON0, g_AM_BSP_GPIO_BUTTON0);
    }
    
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    AM_CRITICAL_END

}

void BUTTONS_DRIVER_disable_interrupts() {
        am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                            AM_HAL_GPIO_INT_CTRL_INDV_DISABLE,
                            (void *)&ui32BUTTON0GpioNum);
}

void BUTTONS_DRIVER_enable_interrupts() {
        uint32_t ui32IntStatus;
        AM_CRITICAL_BEGIN
        am_hal_gpio_interrupt_irq_status_get(GPIO0_001F_IRQn, false, &ui32IntStatus);
        am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
        AM_CRITICAL_END

        //
        // Enable the GPIO/button interrupt.
        //
        am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                            AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                            (void *)&ui32BUTTON0GpioNum);
}

void BUTTONS_DRIVER_initialize() {
    am_devices_button_array_init(am_bsp_psButtons, AM_BSP_NUM_BUTTONS);

    //
    // Configure the button pin.
    //

    am_hal_gpio_pinconfig(AM_BSP_GPIO_BUTTON0, g_AM_BSP_GPIO_BUTTON0);
    //
    // Clear the GPIO Interrupt (write to clear).
    //

    BUTTONS_DRIVER_enable_interrupts();


    NVIC_SetPriority(GPIO0_001F_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(GPIO0_001F_IRQn);

}
