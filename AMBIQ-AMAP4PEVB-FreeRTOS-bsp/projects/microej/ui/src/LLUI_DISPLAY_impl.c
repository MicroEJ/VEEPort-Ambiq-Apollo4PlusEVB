/*
 * C
 *
 * Copyright 2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include "LLUI_DISPLAY_impl.h"

#include "ui_buffer.h"
#include "ui_drawing.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "trace_platform.h"
#include "am_devices_dsi_rm67162.h"
#include "nema_event.h"
#include "nema_graphics.h"


#include "ui_drawing_nema.h"
#include "am_bsp.h"


// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

// Print to UART the framerate
// #define COMPUTE_FRAMERATE

#ifdef SYSTEM_VIEW

/*
 * Trace start macros
 */
#define TRACE_START(event) \
		TRACE_PLATFORM_START_VOID(LLUIDISPLAY, LLUIDISPLAY_TRACE_ ## event)

/*
 * Trace start macros
 */
#define TRACE_START_U32(event, v) \
		TRACE_PLATFORM_START_U32(LLUIDISPLAY, LLUIDISPLAY_TRACE_ ## event, v)

/*
 * Trace end macros
 */
#define TRACE_END(event) \
		TRACE_PLATFORM_END_VOID(LLUIDISPLAY, LLUIDISPLAY_TRACE_ ## event);

#else

#define TRACE_START(event)
#define TRACE_START_U32(event, v) 
#define TRACE_END(event)

#endif // SYSTEM_VIEW

/*
 * @brief Identifier to log the waiting of DSI copy
 */
#define LLUIDISPLAY_TRACE_DSI (0)
/*
 * @brief Identifier to log the GPU configuration (update of framebuffer)
 */
#define LLUIDISPLAY_TRACE_GPU (1)
/*
 * @brief Identifier to log the waiting of GPU execution
 */
#define LLUIDISPLAY_TRACE_GPU_WAIT (2)

#if DEBUG_COMMAND_LIST_ALLOCATION
misra-c2012-8.9
static uint32_t max_sizeof_command_list = 0;
#endif

#define MICROEJ_GPU_TASK_PRIORITY 5

#define GPU_STACK_SIZE       (1 * 1024)
#define GPU_TASK_PRIORITY    (MICROEJ_GPU_TASK_PRIORITY)
#define GPU_TASK_STACK_SIZE  (GPU_STACK_SIZE / 4)

#define DISPLAY_FLUSH_TASK_STACK_SIZE ((1*1024) / 4)
#define DISPLAY_FLUSH_TASK_PRIORITY MICROEJ_GPU_TASK_PRIORITY

#define FRAME_BUFFERS_NUMBER  1

static nemadc_layer_t back_buffer_layer;
static nemadc_layer_t frame_buffers_layers[FRAME_BUFFERS_NUMBER] = {{0}};

static SemaphoreHandle_t sync_gpu;
static SemaphoreHandle_t sync_flush;
static nema_cmdlist_t* gpu_command_list;

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------


#if DEBUG_COMMAND_LIST_ALLOCATION
static uint32_t _sizeof_command_list(nema_cmdlist_t* cl)
{
	uint32_t size = cl->bo.size;
	uint8_t* addr = (uint8_t*)(cl->bo.base_phys + size - 1);
	while(addr != (uint8_t*)cl->bo.base_phys)
	{
		if (*addr != 0)
		{
			break;
		}
		--size;
		--addr;
	}
	return size;
}
#endif

/*
 * @brief: Task to manage display flushes and synchronize with hardware rendering
 * operations
 *
 * note: cannot share this task with drawing_nema's stack because the call to
 * WT0302A2_DsiSendSingleFrame is blocker. During this call, the end of GPU drawings
 * cannot be notified until the DSI copy is performed.
 */
static void __display_task(void * pvParameters)
{
	(void) pvParameters;
	while(1)
	{
		// wait for the request "application flush"
		xSemaphoreTake(sync_flush, portMAX_DELAY);
                
		// start & wait the copy from back buffer to frame buffer (GPU copy)
		nema_cl_submit(gpu_command_list);
		nema_cl_wait(gpu_command_list);

		// application is able to draw in new backbuffer now

		// start and wait end of DSI copy
		nemadc_set_layer(0, &back_buffer_layer); // update DC peripheral
		dsi_send_frame_single(NEMADC_OUTP_OFF);
                
		LLUI_DISPLAY_flushDone(false);
		#ifdef COMPUTE_FRAMERATE
		nema_calculate_fps();
		#endif
	}
}

/*
 * @brief: Task to manage display flushes and synchronize with hardware rendering
 * operations
 *
 * note: cannot share this task with llui_display's stack because the call to
 * WT0302A2_DsiSendSingleFrame is blocker. During this call, the end of GPU drawings
 * cannot be notified until the DSI copy is performed.
 */
static void __gpu_task(void * pvParameters)
{
	(void) pvParameters;
	while(1)
	{
		xSemaphoreTake(sync_gpu, portMAX_DELAY);

		// wait GPU
		nema_cl_wait(gpu_command_list);

		// update flush limits
		int minx;
		int miny;
		int maxx;
		int maxy;
		nema_get_dirty_region(&minx, &miny, &maxx, &maxy);
		(void) LLUI_DISPLAY_setDrawingLimits(minx, miny, maxx, maxy);

#if DEBUG_COMMAND_LIST_ALLOCATION
		uint32_t command_list_size = _sizeof_command_list(gpu_command_list);
		if (command_list_size > max_sizeof_command_list)
		{
			am_util_stdio_printf("command list size = %u\n",  command_list_size);
			max_sizeof_command_list = command_list_size;
		}
#endif

		// notify the Graphics Engine
		LLUI_DISPLAY_notifyAsynchronousDrawingEnd(false);
	}
}

void LLUI_DISPLAY_IMPL_binarySemaphoreTake(void* binary_semaphore) {
	xSemaphoreTake((SemaphoreHandle_t) binary_semaphore, portMAX_DELAY);
}

void LLUI_DISPLAY_IMPL_binarySemaphoreGive(void* binary_semaphore, bool from_isr) {
	if (from_isr) {
		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR((SemaphoreHandle_t) binary_semaphore, &xHigherPriorityTaskWoken);
		if (pdFALSE != xHigherPriorityTaskWoken) {
			// Force a context switch here.
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
	else {
		xSemaphoreGive((SemaphoreHandle_t) binary_semaphore);
	}
}

/*
 * @brief Submit the GPU commands list and wake up the GPU task.
 * @return #DRAWING_RUNNING.
 */
static DRAWING_Status _nemacmdlist_submit(void)
{
	nema_cl_submit(gpu_command_list);
	xSemaphoreGive(sync_gpu);
	return DRAWING_RUNNING;
}


/*
 * @brief Initialze a Nema layer.
 */
static void _layer_initialize(nemadc_layer_t* layer, nema_tex_format_t format, uint32_t buffer_size)
{       
	nema_buffer_t buffer = nema_buffer_create(buffer_size);
	layer->sizex		= DISPLAY_WIDTH;
	layer->resx 		= DISPLAY_WIDTH;
	layer->sizey		= DISPLAY_HEIGHT;
	layer->resy 		= DISPLAY_HEIGHT;
	layer->stride		= -1;
	switch(format)
	{
	case NEMA_RGB565: layer->format = NEMADC_RGB565; break;
	case NEMA_TSC4: layer->format = NEMADC_TSC4; break;
	case NEMA_TSC6: layer->format = NEMADC_TSC6; break;
	case NEMA_TSC6A: layer->format = NEMADC_TSC6A; break;
	default: layer->format = NEMADC_RGB565; break;
	}
	layer->blendmode     = NEMADC_BL_DST_OVER;
	layer->baseaddr_phys = buffer.base_phys;
	layer->baseaddr_virt = buffer.base_virt;
}


/*
 * @brief Initialze the display.
 */
static void _init_screen(void) {
	if (0 == nemadc_init()) {

		uint8_t num_lanes = g_sDsiCfg.ui8NumLanes;
		uint8_t dbi_width = g_sDsiCfg.eDbiWidth;
		uint32_t freq_trim = g_sDsiCfg.eDsiFreq;

		uint32_t mipi_cfg;
		
		if ((uint8_t)16 == dbi_width) {
			mipi_cfg = MIPICFG_16RGB888_OPT0;
		}
		else if ((uint8_t)8 == dbi_width) {
			mipi_cfg = MIPICFG_8RGB888_OPT0;
		}
		else {
			mipi_cfg = 0;
		}

		if (((uint32_t)0 == am_hal_dsi_para_config(num_lanes, dbi_width, freq_trim)) && ((uint32_t)0 != mipi_cfg)) {
			am_devices_dsi_rm67162_init(mipi_cfg, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, 0);
		}

	}
}

// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

void LLUI_DISPLAY_IMPL_initialize(LLUI_DISPLAY_SInitData* init_data)
{
	gpu_command_list = UI_DRAWING_NEMA_initialize(&_nemacmdlist_submit);
	_init_screen();

	// configure layers used by the DC peripheral
	_layer_initialize(&back_buffer_layer, NEMA_RGB565, DISPLAY_WIDTH*DISPLAY_HEIGHT*2);
	for(int i = 0; i < FRAME_BUFFERS_NUMBER; i++)
	{
		_layer_initialize(&frame_buffers_layers[i], FRAME_BUFFER_FORMAT, FRAME_BUFFER_SIZE);
	}

	nema_event_init(1, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	nema_wait_irq();

	// MicroUI Graphics Engine initialization
	init_data->binary_semaphore_0 = (void*)xSemaphoreCreateBinary();
	init_data->binary_semaphore_1 = (void*)xSemaphoreCreateBinary();
	init_data->lcd_width = DISPLAY_WIDTH;
	init_data->lcd_height = DISPLAY_HEIGHT;
	init_data->back_buffer_address = (uint8_t*) back_buffer_layer.baseaddr_phys;

	sync_gpu = xSemaphoreCreateBinary();
	BaseType_t ret = xTaskCreate(
			__gpu_task,
			"GPU",
			GPU_TASK_STACK_SIZE,
			NULL,
			GPU_TASK_PRIORITY,
			NULL);

	sync_flush = xSemaphoreCreateBinary();
	xTaskCreate(
			__display_task,
			"Display",
			DISPLAY_FLUSH_TASK_STACK_SIZE,
			NULL,
			DISPLAY_FLUSH_TASK_PRIORITY,
			NULL);
}

uint8_t* LLUI_DISPLAY_IMPL_flush(MICROUI_GraphicsContext* gc, uint8_t* addr, uint32_t xmin, uint32_t ymin, uint32_t xmax, uint32_t ymax)
{
	(void) gc;
	TRACE_START(GPU);
        
	addr = UI_DRAWING_NEMA_flush(addr, frame_buffers_layers[0].baseaddr_phys, xmin, ymin, xmax, ymax);
		// wakeup the display task
	xSemaphoreGive(sync_flush);
	TRACE_END(GPU);

	return addr;
}
