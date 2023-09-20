/*
 * C
 *
 * Copyright 2021-2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_devices_mspi_psram_aps12808l.h"

#include "microej_main.h"
#include "power.h"

#include "FreeRTOS.h"
#include "task.h"

#ifdef SYSTEM_VIEW
#include "SEGGER_SYSVIEW.h"
#include "SEGGER_RTT.h"
#include "trace_platform.h"
#endif

/* Private define ------------------------------------------------------------*/
#define MICROEJ_CORE_ENGINE_TASK_STACK_SIZE      (5 * 1024)
#define JAVA_TASK_PRIORITY       ( 4 ) /** Should be > tskIDLE_PRIORITY & < configTIMER_TASK_PRIORITY */
#define JAVA_TASK_STACK_SIZE     MICROEJ_CORE_ENGINE_TASK_STACK_SIZE/4


#ifdef MICROEJ_CORE_VALIDATION
#include "t_core_main.h"
#endif


static void xJavaTaskFunction(void * pvParameters)
{
	(void) pvParameters;
	/* Start the VM */
	microej_main(0, NULL);

	/* Should reach this point only if the VM exits */
	vTaskDelete(xTaskGetCurrentTaskHandle());
}

static void          *pDevHandle;
static void          *pHandle;
static uint32_t        DMATCBBuffer[2560];
#define MSPI_TEST_MODULE              0

static am_devices_mspi_psram_config_t MSPI_PSRAM_OctalCE0MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

int main(void) {
	am_util_id_t sIdDevice;

	// Switch the external power on.
	am_bsp_external_pwr_on();

	//
	// Configure the board for low power operation.
	//
	low_power_init();

#ifdef HIGH_PERFORMANCE_MODE
	/* Switch to high performance mode.
	 * Set the clock to 192 MHz instead of 96 MHz.
	 */
	am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE);
#endif

	//
	// Initialize the printf interface for UART output
	//
	if (am_bsp_uart_printf_enable() != 0) {
		// Cannot print - so no point proceeding
		while (1){};
	}
	NVIC_DisableIRQ((IRQn_Type) (UART0_IRQn + AM_BSP_UART_PRINT_INST));

	am_util_stdio_terminal_clear();

	//
	// Print the device info.
	//
	am_util_id_device(&sIdDevice);
	am_util_stdio_printf("Vendor Name: %s\n", sIdDevice.pui8VendorName);
	am_util_stdio_printf("Device type: %s\n", sIdDevice.pui8DeviceName);
	am_util_stdio_printf("Qualified: %s\n",
	                     sIdDevice.sMcuCtrlDevice.ui32Qualified ?
	                     "Yes" : "No");
	am_util_stdio_printf("Device Info:\n"
	                     "\tPart number: 0x%08X\n"
	                     "\tChip ID0:    0x%08X\n"
	                     "\tChip ID1:    0x%08X\n"
	                     "\tRevision:    0x%08X (Rev%c%c)\n",
	                     sIdDevice.sMcuCtrlDevice.ui32ChipPN,
	                     sIdDevice.sMcuCtrlDevice.ui32ChipID0,
	                     sIdDevice.sMcuCtrlDevice.ui32ChipID1,
	                     sIdDevice.sMcuCtrlDevice.ui32ChipRev,
	                     sIdDevice.ui8ChipRevMaj, sIdDevice.ui8ChipRevMin );
        
	// Check the display configuration. Abort if the interface is not set to DSI.
	if (IF_DSI != g_sDispCfg[g_eDispType].eInterface) {
		while (1){};
	}

	// Power the external VDD1.8V switch
	am_hal_dsi_register_external_vdd18_callback(am_bsp_external_vdd18_switch);

	// Power the DSI and configure its clock
	am_hal_dsi_init();

	// Power peripherals
	am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
	am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);
        
	am_hal_interrupt_master_enable();

#ifdef MICROEJ_CORE_VALIDATION
	T_CORE_main();
#else
	/* Start MicroJvm task */
	TaskHandle_t pvCreatedTask;
	xTaskCreate(xJavaTaskFunction, "MicroJvm", JAVA_TASK_STACK_SIZE, NULL, JAVA_TASK_PRIORITY, &pvCreatedTask);

#ifdef SYSTEM_VIEW
	SEGGER_SYSVIEW_Conf();
	am_util_stdio_printf("SEGGER_RTT block address: 0x%x\n", (uint32_t)&(_SEGGER_RTT));
	TRACE_PLATFORM_initialize();

	SEGGER_SYSVIEW_setMicroJVMTask((U32)pvCreatedTask);
        //SEGGER_SYSVIEW_Start();
        
#endif //SYSTEM_VIEW
#endif //MICROEJ_CORE_VALIDATION

	// Start the scheduler.
	vTaskStartScheduler();
	while (1){};

}
