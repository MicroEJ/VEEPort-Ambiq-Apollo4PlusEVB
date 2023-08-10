/*
 * C
 *
 * Copyright 2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
#if !defined UI_DRAWING_NEMA_H
#define UI_DRAWING_NEMA_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "nema_cmdlist.h"
#include "ui_drawing.h"
#include "ui_buffer.h"

// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------

/*
 * @typedef Type abreviation for the Nema callback function
 */
typedef DRAWING_Status (*submitNemaCallback)(void);

#define FRAME_BUFFER_FORMAT (NEMA_RGB565) // NEMA_RGB565 | NEMA_TSC4 | NEMA_TSC6 | NEMA_TSC6A

// TSI malloc size
#if FRAME_BUFFER_FORMAT == NEMA_TSC4
#define FRAME_BUFFER_SIZE (DISPLAY_WIDTH/4 * DISPLAY_HEIGHT/4 * 64/8) // 64 bits per 4x4 block of pixels
#elif FRAME_BUFFER_FORMAT == NEMA_TSC6
#define FRAME_BUFFER_SIZE (DISPLAY_WIDTH/4 * DISPLAY_HEIGHT/4 * 96/8) // 96 bits per 4x4 block of pixels
#elif FRAME_BUFFER_FORMAT == NEMA_TSC6A
#define FRAME_BUFFER_SIZE (DISPLAY_WIDTH/4 * DISPLAY_HEIGHT/4 * 96/8) // 96 bits per 4x4 block of pixels
#elif FRAME_BUFFER_FORMAT == NEMA_RGB565
#define FRAME_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT * 2)
#else
#error "Invalid FrameBuffer format"
#endif

/*
 * @brief To draw a shape, the GPU uses the command list. For rectangular shapes
 * (draw/fill rectangles and images), the max list size is fixed (around 300 bytes).
 *
 * For the other shapes (circle, etc.) the list increases according the shape' size
 * (dynamic shape): several blocks of 1024 bytes and 40 bytes are allocated and never
 * freed (set breakpoints on assembler functions "tsi_malloc_pool" and "tsi_free").
 *
 * Note: "nema_draw_circle" internally calls "nema_draw_rounded_rect" that consumes the
 * 1024 & 40 bytes blocks.
 *
 * By default the dynamic shapes are disabled and the software algorithms are used
 * instead.
 *
 * To enable these drawings, the command list size must be calibrated.
 */
#ifndef ENABLE_DYNAMIC_SHAPES
#define ENABLE_DYNAMIC_SHAPES (0)
#endif

/*
 * @brief The software algorithms are faster than GPU to draw horizontal and vertical
 * lines; and by consequence to draw rectangle.
 *
 * By default these shapes are disabled and the software algorithms are used
 * instead.
 */
#ifndef ENABLE_SIMPLE_LINES
#define ENABLE_SIMPLE_LINES (0)
#endif

/*
 * @brief Measures the memory consumption in the command list and prints a debug line
 * when this consumption is bigger than the previous one.
 */
#ifndef DEBUG_COMMAND_LIST_ALLOCATION
#define DEBUG_COMMAND_LIST_ALLOCATION (0)
#endif


/*
 * The fonctions UI_DRAWING_NEMA_xxx() are directly called by LLUI_PAINTER_impl.c
 * and LLDW_PAINTER_impl.c. This file overrides each function independently to use the
 * NEMA GPU.
 */

#define UI_DRAWING_NEMA_drawLine UI_DRAWING_drawLine

#if ENABLE_SIMPLE_LINES

#define UI_DRAWING_NEMA_drawHorizontalLine UI_DRAWING_drawHorizontalLine
#define UI_DRAWING_NEMA_drawVerticalLine UI_DRAWING_drawVerticalLine
#define UI_DRAWING_NEMA_drawRectangle UI_DRAWING_drawRectangle

#endif // ENABLE_SIMPLE_LINES

#define UI_DRAWING_NEMA_fillRectangle UI_DRAWING_fillRectangle


#if ENABLE_DYNAMIC_SHAPES

#define UI_DRAWING_NEMA_drawRoundedRectangle UI_DRAWING_drawRoundedRectangle
#define UI_DRAWING_NEMA_fillRoundedRectangle UI_DRAWING_fillRoundedRectangle
#define UI_DRAWING_NEMA_drawCircle UI_DRAWING_drawCircle
#define UI_DRAWING_NEMA_fillCircle UI_DRAWING_fillCircle

#endif // ENABLE_DYNAMIC_SHAPES

#define UI_DRAWING_NEMA_drawImage UI_DRAWING_drawImage
#define UI_DRAWING_NEMA_drawRegion UI_DRAWING_drawRegion
#define UI_DRAWING_NEMA_copyImage UI_DRAWING_copyImage

#define DW_DRAWING_NEMA_drawRotatedImageBilinear DW_DRAWING_drawRotatedImageBilinear
#define DW_DRAWING_NEMA_drawRotatedImageNearestNeighbor DW_DRAWING_drawRotatedImageNearestNeighbor
#define DW_DRAWING_NEMA_drawScaledImageBilinear DW_DRAWING_drawScaledImageBilinear
#define DW_DRAWING_NEMA_drawScaledImageNearestNeighbor DW_DRAWING_drawScaledImageNearestNeighbor



// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

/*
 * @brief Initializes the GPU drawer.
 * @param[in] func the NEMA submit callback function
 * @return Address of the NEMA command list
 */
nema_cmdlist_t* UI_DRAWING_NEMA_initialize(submitNemaCallback func);

/*
 * @brief Performs a GPU flush.
 *
 * @param[in] addr the address of graphics context buffer
 * @param[in] dst the address of frame buffer buffer
 * @param[in] xmin the top-left X-coordinate of the dirty area.
 * @param[in] ymin the top-left Y-coordinate of the dirty area.
 * @param[in] xmax the bottom-right X-coordinate of the dirty area.
 * @param[in] ymax the bottom-right Y-coordinate of the dirty area.
 * 
 * @return the new graphics buffer address
 */
uint8_t* UI_DRAWING_NEMA_flush(uint8_t* addr, uintptr_t dst, uint32_t xmin, uint32_t ymin, uint32_t xmax, uint32_t ymax);


// --------------------------------------------------------------------------------
// ui_drawing.h API
// --------------------------------------------------------------------------------

/*
 * @brief Implementation of drawLine over VG-Lite. See ui_drawing.h
 */
DRAWING_Status UI_DRAWING_NEMA_drawLine(MICROUI_GraphicsContext* gc, jint startX, jint startY, jint endX, jint endY);

#if ENABLE_SIMPLE_LINES
/*
 * @brief Implementation of drawHorizontalLine over VG-Lite. See ui_drawing.h
 */
DRAWING_Status UI_DRAWING_NEMA_drawHorizontalLine(MICROUI_GraphicsContext* gc, jint x1, jint x2, jint y);

/*
 * @brief Implementation of drawVerticalLine over VG-Lite. See ui_drawing.h
 */
DRAWING_Status UI_DRAWING_NEMA_drawVerticalLine(MICROUI_GraphicsContext* gc, jint x, jint y1, jint y2);

/*
 * @brief Implementation of drawRectangle over VG-Lite. See ui_drawing.h
 */
DRAWING_Status UI_DRAWING_NEMA_drawRectangle(MICROUI_GraphicsContext* gc, jint x1, jint y1, jint x2, jint y2);

#endif // ENABLE_SIMPLE_LINES

/*
 * @brief Implementation of fillRectangle over VG-Lite. See ui_drawing.h
 */
DRAWING_Status UI_DRAWING_NEMA_fillRectangle(MICROUI_GraphicsContext* gc, jint x1, jint y1, jint x2, jint y2);


#if ENABLE_DYNAMIC_SHAPES


/*
 * @brief Implementation of drawRoundedRectangle over VG-Lite. See ui_drawing.h
 */
DRAWING_Status UI_DRAWING_NEMA_drawRoundedRectangle(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height, jint cornerEllipseWidth, jint cornerEllipseHeight);


/*
 * @brief Implementation of fillRoundedRectangle over VG-Lite. See ui_drawing.h
 */
DRAWING_Status UI_DRAWING_NEMA_fillRoundedRectangle(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height, jint cornerEllipseWidth, jint cornerEllipseHeight);

/*
 * @brief Implementation of drawCircle over VG-Lite. See ui_drawing.h
 */
DRAWING_Status UI_DRAWING_NEMA_drawCircle(MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter);

/*
 * @brief Implementation of fillCircle over VG-Lite. See ui_drawing.h
 */
DRAWING_Status UI_DRAWING_NEMA_fillCircle(MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter);

#endif // ENABLE_DYNAMIC_SHAPES


/*
 * @brief Implementation of drawImage over VG-Lite. See ui_drawing.h
 */
DRAWING_Status UI_DRAWING_NEMA_drawImage(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint regionX, jint regionY, jint width, jint height, jint x, jint y, jint alpha);

/*
 * @brief Implementation of copyImage over VG-Lite. See ui_drawing.h
 */
DRAWING_Status UI_DRAWING_NEMA_copyImage(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint regionX, jint regionY, jint width, jint height, jint x, jint y);

/*
 * @brief Implementation of drawRegion over VG-Lite. See ui_drawing.h
 */
DRAWING_Status UI_DRAWING_NEMA_drawRegion(MICROUI_GraphicsContext* gc, jint x_src, jint y_src, jint width, jint height, jint x_dest, jint y_dest, jint alpha);


// --------------------------------------------------------------------------------
// dw_drawing.h API
// --------------------------------------------------------------------------------

/*
 * @brief Implementation of drawRotatedImageBilinear over VG-Lite. See ui_drawing.h
 */
DRAWING_Status DW_DRAWING_NEMA_drawRotatedImageBilinear(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jint rotationX, jint rotationY, jfloat angle, jint alpha);

/*
 * @brief Implementation of drawRotatedImageNearestNeighbor over VG-Lite. See ui_drawing.h
 */
DRAWING_Status DW_DRAWING_NEMA_drawRotatedImageNearestNeighbor(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jint rotationX, jint rotationY, jfloat angle, jint alpha);

/*
 * @brief Implementation of drawScaledImageBilinear over VG-Lite. See ui_drawing.h
 */
DRAWING_Status DW_DRAWING_NEMA_drawScaledImageBilinear(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jfloat factorX, jfloat factorY, jint alpha);

/*
 * @brief Implementation of drawScaledImageNearestNeighbor over VG-Lite. See ui_drawing.h
 */
DRAWING_Status DW_DRAWING_NEMA_drawScaledImageNearestNeighbor(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jfloat factorX, jfloat factorY, jint alpha);

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#endif // !defined UI_DRAWING_NEMA_H
