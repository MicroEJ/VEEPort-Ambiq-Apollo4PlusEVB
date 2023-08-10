/*
 * C
 *
 * Copyright 2021-2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "FreeRTOSConfig.h"

#include "power.h"

void low_power_init(void)
{

      // Bsp low power init
	  am_bsp_low_power_init();

	  //
	  // Power down Crypto.
	  //
	  am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_CRYPTO_POWERDOWN, 0);

	  //
	  // Disable all peripherals
	  //
	  am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_DIS_PERIPHS_ALL, 0);


	  //
	  // Disable XTAL in deepsleep
	  //
      if(configSTIMER_SRC != AM_HAL_STIMER_XTAL_32KHZ){
    	  am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_XTAL_PWDN_DEEPSLEEP, 0);
      }

	  // Retain all DTCM and SSRAM but not Extended RAM
	  // Memory config for DTCM memory
	  am_hal_pwrctrl_mcu_memory_config_t McuMemCfg =
	  {
	    .eCacheCfg = AM_HAL_PWRCTRL_CACHE_ALL,   //AM_HAL_PWRCTRL_CACHE_NONE,
	    .bRetainCache = true,                    //true  false
	    .eDTCMCfg = AM_HAL_PWRCTRL_DTCM_384K,    //AM_HAL_PWRCTRL_DTCM_8K,
	    .eRetainDTCM = AM_HAL_PWRCTRL_DTCM_384K, //AM_HAL_PWRCTRL_DTCM_384K,
	    .bEnableNVM0 = true,
	    .bRetainNVM0 = false
	  };
	  // Config SRAM
	  am_hal_pwrctrl_sram_memcfg_t SRAMMemCfg =
	  {
	    .eSRAMCfg = AM_HAL_PWRCTRL_SRAM_1M,
	    .eActiveWithMCU = AM_HAL_PWRCTRL_SRAM_NONE,
	    .eActiveWithDSP = AM_HAL_PWRCTRL_SRAM_NONE,
	    .eSRAMRetain = AM_HAL_PWRCTRL_SRAM_1M //AM_HAL_PWRCTRL_SRAM_NONE  //AM_HAL_PWRCTRL_SRAM_1M
	  };

	  //
	  // Update memory configuration to minimum.
	  //
	  am_hal_pwrctrl_mcu_memory_config(&McuMemCfg);
	  am_hal_pwrctrl_sram_config(&SRAMMemCfg);


	//Config Extended SRAM]
	  am_hal_pwrctrl_dsp_memory_config_t ExtSRAMMemCfg =
	  {
		  .bEnableICache = false,
		  .bRetainCache = false,
		  .bEnableRAM = true,  //true enables Extended RAM
		  .bActiveRAM = false, //Recommended default setting should is "false"
		  .bRetainRAM = false   //false configures Extended RAM as not retained in deep sleep
	  };
	  am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP0, &ExtSRAMMemCfg);
	  am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP1, &ExtSRAMMemCfg);

	  //
	  // Set the default cache configuration
	  //
	  am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
	  am_hal_cachectrl_enable();


	  am_hal_pwrctrl_dsp_mode_select(AM_HAL_DSP0, AM_HAL_PWRCTRL_DSP_MODE_ULTRA_LOW_POWER);
	  am_hal_pwrctrl_dsp_mode_select(AM_HAL_DSP1, AM_HAL_PWRCTRL_DSP_MODE_ULTRA_LOW_POWER);

	 //
	 // Go to Deep Sleep and stay there.
	 // Uncomment to measure deep sleep current
	 //
	 //am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
}

uint32_t am_freertos_sleep(uint32_t time)
{
	(void) time;
	//am_util_stdio_printf("2 DEVPWRSTATUS 0x%x\n", PWRCTRL->DEVPWRSTATUS);
	//am_util_stdio_printf("2 DEVPWREN 0x%x\n", PWRCTRL->DEVPWREN);

	// Disable uart before going to sleep
	//am_bsp_buffered_uart_printf_disable();

	//
	// Go to Deep Sleep and wait for a wake up.
	//
	am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
	return 0; // avoid sleeping again
}

void am_freertos_wakeup(uint32_t time)
{
	(void) time;
	// Reenable uart going out of sleep
	//if (am_bsp_buffered_uart_printf_enable())
	//{
	//    // Cannot print - so no point proceeding
	//    while(1);
	//}
	NVIC_DisableIRQ((IRQn_Type)(UART0_IRQn + AM_BSP_UART_PRINT_INST));
}
