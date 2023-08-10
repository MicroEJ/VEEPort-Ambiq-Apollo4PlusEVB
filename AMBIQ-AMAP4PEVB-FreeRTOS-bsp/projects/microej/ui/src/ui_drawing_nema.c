/*
 * C
 *
 * Copyright 2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <LLUI_DISPLAY.h>

#include "ui_drawing_nema.h"
#include "ui_drawing_soft.h"
#include "dw_drawing.h"
#include "dw_drawing_soft.h"

// BSP
#include "nema_blender.h"
#include "nema_matrix3x3.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "am_devices_dsi_rm67162.h"
#include "am_bsp.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

#define UI_ERROR_BYPASS


#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))



// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------

static nema_cmdlist_t command_list;

static submitNemaCallback submitCallback;
// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

/*
 * @brief Converts a ARGB8888 color in NEMA color format ABGR8888
 */
static uint32_t _convert_color(uint32_t c) {
	return (c & (uint32_t)0xff00ff00U) | ((c & (uint32_t)0xff0000u) >> 16) | ((c & (uint32_t)0xff) << 16);

}

/*
 * @brief Resets the GPU commands list with the new destination (RGB565 format for sure).
 * Returns false when there is nothing to draw.
 * Updates nema_color with graphics context foreground color (in Nema GPU format)
 */
static bool _nemacmdlist_reset(MICROUI_GraphicsContext* gc, int x1, int y1, int x2, int y2, uint32_t* nema_color) {
	bool ret = true;

	// clear command list
	nema_cl_rewind(&command_list);
	nema_cl_bind(&command_list);
	nema_clear_dirty_region();

	if (LLUI_DISPLAY_isClipEnabled(gc))
	{
		if (!LLUI_DISPLAY_clipRectangle(gc, &x1, &y1, &x2, &y2))
		{
			// drawing is fully outside the clip, nothing to draw
			ret = false;
		}

		// drawing fully or partially fits the clip: enable clip
		nema_set_clip(gc->clip_x1, gc->clip_y1, gc->clip_x2-gc->clip_x1+1, gc->clip_y2-gc->clip_y1+1);
	}
	else
	{
		// clip is disabled: reset clip
		nema_set_clip(0, 0, gc->image.width, gc->image.height);
	}

	if (ret)
	{
		// configure destination
		nema_bind_dst_tex(
				(uintptr_t)LLUI_DISPLAY_getBufferAddress(&gc->image),
				gc->image.width,
				gc->image.height,
				NEMA_RGB565,
				LLUI_DISPLAY_getStrideInBytes(&gc->image)
		);

		// retrieve the color to use
		*nema_color = _convert_color(gc->foreground_color);
	}

	// perform drawing
	return ret;
}


/*
 * @brief Configure the image source if the image format is supported by the GPU.
 * @return True is the image format is supported by the GPU
*/
static bool _nemacmdlist_configure_source(MICROUI_Image* img, uint32_t nema_color, uint32_t alpha, nema_tex_mode_t mode) {
	nema_tex_format_t format;
	uint32_t blending_mode;
	bool ret = true;

	switch(img->format)
	{
	case MICROUI_IMAGE_FORMAT_ARGB8888:
		format = NEMA_BGRA8888;
		blending_mode = NEMA_BL_SIMPLE;
		break;
	case MICROUI_IMAGE_FORMAT_RGB565:
		format = NEMA_RGB565;
		blending_mode = NEMA_BL_SRC_OVER;
		break;
	case MICROUI_IMAGE_FORMAT_A8:
		format = NEMA_A8;
		blending_mode = NEMA_BL_SIMPLE;
		nema_set_tex_color(nema_color);
		break;

	case MICROUI_IMAGE_FORMAT_A4:
	case MICROUI_IMAGE_FORMAT_A2:
	case MICROUI_IMAGE_FORMAT_A1:
		// [MicroEJ versus Nema] pixels are not in the same way in the byte

	case MICROUI_IMAGE_FORMAT_ARGB1555:
	case MICROUI_IMAGE_FORMAT_ARGB4444:
		// [MicroEJ versus Nema] blue and red are inverted

	default:
		ret = false;
		break;
	}

	if (ret)
	{
		nema_set_blend_blit (blending_mode | NEMA_BLOP_MODULATE_A);
		nema_set_const_color(alpha << 24);
		nema_bind_src_tex ( (uintptr_t)LLUI_DISPLAY_getBufferAddress(img) ,
				img->width, img->height,
				format ,
				LLUI_DISPLAY_getStrideInBytes(img),
				mode );
	}

	return ret;
}


/*
 * @brief Rotates an image according given nema_tex_mode_t.
 * @return true when drawing is done/started; false when caller has to perform the
 * drawing in software.
 */

static bool _rotate_image(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jint rotationX, jint rotationY, jfloat angle, jfloat factorX, jfloat factorY, jint alpha, nema_tex_mode_t mode, DRAWING_Status* status) {
	bool ret = true;
	// checks if there is something to draw and clip drawing limits
	// TODO try to crop region
	uint32_t color;
	if (!_nemacmdlist_reset(gc, 0, 0, gc->image.width, gc->image.height, &color))
	{
		*status = DRAWING_DONE;
	}

	// configure GPU with the image to render
	if (!_nemacmdlist_configure_source(img, color, alpha, mode))
	{
		ret = false;
	}
	else
	{
		// nema_blit_quad_fit reference point is image's centered point
		float x0 = -img->width/2;
		float y0 = -img->height/2;
		float x1 = x0 + img->width;
		float y1 = y0;
		float x2 = x0 + img->width;
		float y2 = y0 + img->height;
		float x3 = x0;
		float y3 = y0 + img->height;

		// calculate rotation matrix
		// (x,y) is the image's position when there is no transformation (top-left anchor)
		nema_matrix3x3_t m;
		nema_mat3x3_load_identity(m);
		nema_mat3x3_scale(m, factorX, factorY);
		nema_mat3x3_translate(m, x + (img->width * factorX) / 2 - rotationX , y + (img->height * factorY) / 2 - rotationY);
		nema_mat3x3_rotate(m, -angle);
		nema_mat3x3_translate(m, rotationX, rotationY);	

		// apply matrix
		nema_mat3x3_mul_vec(m, &x0, &y0);
		nema_mat3x3_mul_vec(m, &x1, &y1);
		nema_mat3x3_mul_vec(m, &x2, &y2);
		nema_mat3x3_mul_vec(m, &x3, &y3);

		// draw image
		nema_blit_quad_fit( x0, y0,
				x1, y1,
				x2, y2,
				x3, y3);

		*status = submitCallback();
	}

	return ret;
}


/*
 * @brief Scales an image according given nema_tex_mode_t.
 * @return true when drawing is done/started; false when caller has to perform the
 * drawing in software.
 */
static bool _scale_image(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jfloat factorX, jfloat factorY, jint alpha, nema_tex_mode_t mode, DRAWING_Status* status) {
	bool ret = true;
	// checks if there is something to draw and clip drawing limits
	// TODO try to crop region
	uint32_t color;
	if (!_nemacmdlist_reset(
			gc, x, y,
			(int) (x + (factorX * img->width)),
			(int) (y + (factorY * img->height)),
			&color))
	{
		*status = DRAWING_DONE;
	}
	else
	{
		// configure GPU with the image to render
		if (!_nemacmdlist_configure_source(img, color, alpha, mode))
		{
			ret = false;
		} 
		else 
		{
			nema_blit_rect_fit(x, y, (int)(img->width * factorX), (int)(img->height * factorY));

			*status = submitCallback();
		}
	}
	return ret;
}



/*
 * @brief Draw in the same image with overlaps.
 * This fuction will slice the drawing in multiple chunk to avoid overlaping drawing.
 * @return the drawing status.
 */
static DRAWING_Status _draw_region(uint32_t rect[4], nema_matrix3x3_t matrix, uint32_t element_index) {
	// rect[x,y,w,h]
	uint32_t rect_index = element_index + (uint32_t)2;

	// retrieve band's size (width or height)
	jint size = rect[rect_index];
	rect[rect_index] = (uint32_t)(matrix[element_index][2]) - rect[element_index];

	// go to x + band size
	rect[element_index] += size;
	// cppcheck-suppress [misra-c2012-10.8] Cast for include in nema matrix.
	matrix[element_index][2] = (float)(rect[element_index] + rect[rect_index]);

	while (size > 0) {

		// adjust band's size
		if (size < rect[rect_index]){
			rect[rect_index] = size;
		}

		// adjust src & dest positions
		rect[element_index] -= rect[rect_index];
		matrix[element_index][2] -= rect[rect_index];

		size -= rect[rect_index];
                
		nema_blit_subrect((int)matrix[0][2] , (int)matrix[1][2] , rect[2], rect[3], rect[0], rect[1]);
                
	}

	return submitCallback();
}



// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
nema_cmdlist_t* UI_DRAWING_NEMA_initialize(submitNemaCallback func) {
	submitCallback = func;
	nema_init();
	
#if ENABLE_DYNAMIC_SHAPES
#warning "GPU's command list size is dynamic !"
	// when the command list cannot be increased, the GPU discards the drawing.
	command_list  = nema_cl_create();
#else
	command_list  = nema_cl_create_sized(0x200); // at least 277 bytes
#endif

	return &command_list;
}

// See the header file for the function documentation
uint8_t* UI_DRAWING_NEMA_flush(uint8_t* src, uintptr_t dst, uint32_t xmin, uint32_t ymin, uint32_t xmax, uint32_t ymax)  {
	// reset GPU commmand list
	nema_cl_rewind(&command_list);
	nema_cl_bind(&command_list);
	nema_set_clip(xmin, ymin, xmax-xmin+(uint32_t)1, ymax-ymin+(uint32_t)1);

	// configure destination (future DSI buffer)
	nema_bind_dst_tex(
			dst,
			DISPLAY_WIDTH, DISPLAY_HEIGHT,
			FRAME_BUFFER_FORMAT,
			-1
	);

	// configure source
	nema_set_blend_blit (NEMA_BL_SRC_OVER);
	// cppcheck-suppress [misra-c2012-11.4] Pointer cast for function signature.
	nema_bind_src_tex ( (uintptr_t)src,
			DISPLAY_WIDTH, DISPLAY_HEIGHT,
			NEMA_RGB565,
			DISPLAY_WIDTH*2,
			NEMA_FILTER_PS
	);

	// prepare the copy of buffer
	nema_blit_subrect(xmin , ymin, xmax-xmin+(uint32_t)1, ymax-ymin+(uint32_t)1, xmin, ymin);

	return src;
}
// -----------------------------------------------------------------------------
// ui_drawing.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
DRAWING_Status UI_DRAWING_NEMA_drawLine(MICROUI_GraphicsContext* gc, jint startX, jint startY, jint endX, jint endY) {
	DRAWING_Status status = DRAWING_DONE;
	// Check if there is something to draw and clip drawing limits
	// TODO try to crop region (x1 may lower than x2 and y1 may be lower than y2)
	uint32_t color;
	if (_nemacmdlist_reset(
			gc,
			0, 0,
			gc->image.width, gc->image.height,
			&color))
	{
		nema_set_blend_fill(NEMA_BL_SRC);
		nema_draw_line(startX, startY, endX, endY, color);

		status = submitCallback();
	}
	return status;
}  


#if ENABLE_SIMPLE_LINES
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_NEMA_drawHorizontalLine(MICROUI_GraphicsContext* gc, jint x1, jint x2, jint y) {
	return UI_DRAWING_NEMA_drawLine(gc, x1, y, x2, y);
}
#endif // ENABLE_SIMPLE_LINES

#if ENABLE_SIMPLE_LINES
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_NEMA_drawVerticalLine(MICROUI_GraphicsContext* gc, jint x, jint y1, jint y2) {
	return UI_DRAWING_NEMA_drawLine(gc, x, y1, x, y2);
}
#endif // ENABLE_SIMPLE_LINES

#if ENABLE_SIMPLE_LINES
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_NEMA_drawRectangle(MICROUI_GraphicsContext* gc, jint x1, jint y1, jint x2, jint y2) {
	DRAWING_Status status = DRAWING_DONE;
	// checks if there is something to draw and clip drawing limits
	uint32_t color;
	if (_nemacmdlist_reset(gc, x1, y1, x2, y2, &color))
	{
		nema_set_blend_fill(NEMA_BL_SRC);
		nema_draw_rect(x1, y1, x2-x1+1, y2-y1+1, color);
		status = submitCallback();
	}
	
	return status;
}

#endif // ENABLE_SIMPLE_LINES

// See the header file for the function documentation
DRAWING_Status UI_DRAWING_NEMA_fillRectangle(MICROUI_GraphicsContext* gc, jint x1, jint y1, jint x2, jint y2) {
	DRAWING_Status status = DRAWING_DONE;
	// checks if there is something to draw and clip drawing limits
	uint32_t color;
	if (_nemacmdlist_reset(gc, x1, y1, x2, y2, &color))
	{

		nema_set_blend_fill(NEMA_BL_SRC);
		nema_fill_rect(x1, y1, x2-x1+1, y2-y1+1, color);
		status = submitCallback();
	}

	return status;
}

#if ENABLE_DYNAMIC_SHAPES
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_NEMA_drawRoundedRectangle(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height, jint cornerEllipseWidth, jint cornerEllipseHeight) {
	DRAWING_Status status = DRAWING_DONE;

	if (cornerEllipseWidth != cornerEllipseHeight)
	{
		// GPU manages only circular angles
		UI_DRAWING_SOFT_drawRoundedRectangle(gc, x, y, width, height,cornerEllipseWidth, cornerEllipseHeight);
	}	
	else
	{
		// checks if there is something to draw and clip drawing limits
		uint32_t color;
		if (_nemacmdlist_reset(gc, x, y, x+width-1, y+height-1, &color))
		{
			nema_set_blend_fill(NEMA_BL_SRC);
			nema_draw_rounded_rect(x, y, width, height, cornerEllipseWidth, color);

			status = submitCallback();
		}
	}

	return status;
}
#endif // ENABLE_DYNAMIC_SHAPES

#if ENABLE_DYNAMIC_SHAPES
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_NEMA_fillRoundedRectangle(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height, jint cornerEllipseWidth, jint cornerEllipseHeight) {
	DRAWING_Status status = DRAWING_DONE;

	if (cornerEllipseWidth != cornerEllipseHeight)
	{
		// GPU manages only circular angles
		UI_DRAWING_SOFT_fillRoundedRectangle(gc, x, y, width, height,cornerEllipseWidth, cornerEllipseHeight);
	}
	else
	{
		// checks if there is something to draw and clip drawing limits
		uint32_t color;
		if (_nemacmdlist_reset(gc, x, y, x+width-1, y+height-1, &color))
		{
			nema_set_blend_fill(NEMA_BL_SRC);
			nema_fill_rounded_rect(x, y, width, height, cornerEllipseWidth, color);

			status = submitCallback();
		}
	}

	return status;
}
#endif // ENABLE_DYNAMIC_SHAPES

#if ENABLE_DYNAMIC_SHAPES
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_NEMA_drawCircle(MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter) {
	DRAWING_Status status = DRAWING_DONE;

	// checks if there is something to draw and clip drawing limits
	uint32_t color;
	if (!_nemacmdlist_reset(gc, x, y, x+diameter-1, y+diameter-1, &color))
	{
		jint radius = diameter / 2;
		nema_set_blend_fill(NEMA_BL_SRC);
		nema_draw_circle(x + radius, y + radius, radius, color);

		status = submitCallback();
	}

	return status;
}
#endif // ENABLE_DYNAMIC_SHAPES

#if ENABLE_DYNAMIC_SHAPES
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_NEMA_fillCircle(MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter) {
	DRAWING_Status status = DRAWING_DONE;

	// checks if there is something to draw and clip drawing limits
	uint32_t color;
	if (_nemacmdlist_reset(gc, x, y, x+diameter-1, y+diameter-1, &color))
	{
		jint radius = diameter / 2;
		nema_set_blend_fill(NEMA_BL_SRC);
		nema_fill_circle(x + radius, y + radius, radius, color);
		status = submitCallback();
	}

	return status;
}
#endif // ENABLE_DYNAMIC_SHAPES

// See the header file for the function documentation
DRAWING_Status UI_DRAWING_NEMA_drawImage(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint regionX, jint regionY, jint width, jint height, jint x, jint y, jint alpha) {
	DRAWING_Status status;

	// checks if there is something to draw and clip drawing limits
	uint32_t color;
	if (!_nemacmdlist_reset(gc, x, y, x + width - 1, y + height - 1, &color))
	{
		status = DRAWING_DONE;
	}
	else
	{
	// configure GPU with the image to render
	if (!_nemacmdlist_configure_source(img, color, alpha, NEMA_FILTER_PS))
	{
		UI_DRAWING_SOFT_drawImage(gc, img, regionX, regionY, width, height, x, y, alpha);
		status = DRAWING_DONE;
	}
        else
        {
          // render the image
          nema_blit_subrect(x , y , width, height, regionX, regionY);
          
          
          status = submitCallback();
        }
	}


	return status;
}

// See the header file for the function documentation
DRAWING_Status UI_DRAWING_NEMA_drawRegion(MICROUI_GraphicsContext* gc, jint x_src, jint y_src, jint width, jint height, jint x_dest, jint y_dest, jint alpha) {
	DRAWING_Status status = DRAWING_DONE;

        uint32_t blit_rect[4];
        
        blit_rect[0] = x_src;
        blit_rect[1] = y_src;
        blit_rect[2] = width;
        blit_rect[3] = height;

        nema_matrix3x3_t matrix;
	nema_mat3x3_load_identity(matrix);
        matrix[0][2] = x_dest;
        matrix[1][2] = y_dest;

        
        
	// checks if there is something to draw and clip drawing limits
	uint32_t color;
	if (_nemacmdlist_reset(gc, x_dest, y_dest, x_dest + width - 1, y_dest + height - 1, &color))
	{
        	// configure GPU with the image to render
	if (_nemacmdlist_configure_source(&gc->image, color, alpha, NEMA_FILTER_PS))
	{
          if ((y_dest == y_src) && (x_dest > x_src) && (x_dest < (x_src + width))){
            // draw with overlap: cut the drawings in several widths
            status = _draw_region(blit_rect, matrix, 0);
          }
          else if ((y_dest > y_src) && (y_dest < (y_src + height))){
            // draw with overlap: cut the drawings in several heights
            status = _draw_region(blit_rect, matrix, 1);
          }
          else {
            // draw in one shot
            nema_blit_subrect(x_dest , y_dest , width, height, x_src, y_src);
            status = submitCallback();
          }
        }
	}
	return status;
}


// See the header file for the function documentation
DRAWING_Status UI_DRAWING_NEMA_copyImage(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint regionX, jint regionY, jint width, jint height, jint x, jint y) {
  	DRAWING_Status status;

		status = (img == &gc->image) ?
				// have to manage the overlap
				UI_DRAWING_NEMA_drawRegion(gc, regionX, regionY, width, height, x, y, 0xff)
				// no overlap: draw image as usual
				: UI_DRAWING_NEMA_drawImage(gc, img, regionX, regionY, width, height, x, y, 0xff);

	return status;
}

// -----------------------------------------------------------------------------
// dw_drawing.h functions
// -----------------------------------------------------------------------------

#if ENABLE_DYNAMIC_SHAPES
// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawThickLine(MICROUI_GraphicsContext* gc, jint x1, jint y1, jint x2, jint y2, jint thickness) {
	DRAWING_Status status = DRAWING_DONE;

	int min_x = MIN(x1, x2);
	int min_y = MIN(y1, y2);
	int max_x = MAX(x1, x2);
	int max_y = MAX(y1, y2);

	// Check if there is something to draw and clip drawing limits
	// TODO try to crop region (x1 may lower than x2 and y1 may be lower than y2)
	uint32_t color;
	if (_nemacmdlist_reset(
			gc,
			min_x - (thickness / 2),
			min_y - (thickness / 2),
			max_x + (thickness / 2),
			max_y + (thickness / 2),
			&color)) {
			nema_set_blend_fill(NEMA_BL_SRC);
			nema_draw_line_aa(x1, y1, x2, y2, thickness, color);
			status = submitCallback();
	}
	return status;
}  

#endif // ENABLE_DYNAMIC_SHAPES

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_NEMA_drawRotatedImageBilinear(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jint rotationX, jint rotationY, jfloat angle, jint alpha) {
	DRAWING_Status status;
#ifdef UI_ERROR_BYPASS
        DW_DRAWING_SOFT_drawRotatedImageBilinear(gc, img, x, y, rotationX, rotationY, angle, alpha);
        status = DRAWING_DONE;
#else
	if (!_rotate_image(gc, img, x, y, rotationX, rotationY, angle, 1.0f, 1.0f, alpha, NEMA_FILTER_BL, &status))
	{
		DW_DRAWING_SOFT_drawRotatedImageBilinear(gc, img, x, y, rotationX, rotationY, angle, alpha);
		status = DRAWING_DONE;
	}
#endif
	return status;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_NEMA_drawRotatedImageNearestNeighbor(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jint rotationX, jint rotationY, jfloat angle, jint alpha) {
	DRAWING_Status status;

#ifdef UI_ERROR_BYPASS
        DW_DRAWING_SOFT_drawRotatedImageNearestNeighbor(gc, img, x, y, rotationX, rotationY, angle, alpha);
        status = DRAWING_DONE;
#else
	if (!_rotate_image(gc, img, x, y, rotationX, rotationY, angle, 1.0f, 1.0f, alpha, NEMA_FILTER_PS, &status))
	{
		DW_DRAWING_SOFT_drawRotatedImageNearestNeighbor(gc, img, x, y, rotationX, rotationY, angle, alpha);
		status = DRAWING_DONE;
	}
#endif
	return status;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_NEMA_drawScaledImageBilinear(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jfloat factorX, jfloat factorY, jint alpha) {
	DRAWING_Status status;
	if (!_scale_image(gc, img, x, y, factorX, factorY, alpha, NEMA_FILTER_BL, &status))
	{
		DW_DRAWING_SOFT_drawScaledImageBilinear(gc, img, x, y, factorX, factorY, alpha);
		status = DRAWING_DONE;
	}
	return status;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_NEMA_drawScaledImageNearestNeighbor(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jfloat factorX, jfloat factorY, jint alpha) {
	DRAWING_Status status;
	if (!_scale_image(gc, img, x, y, factorX, factorY, alpha, NEMA_FILTER_PS, &status))
	{
		DW_DRAWING_SOFT_drawScaledImageNearestNeighbor(gc, img, x, y, factorX, factorY, alpha);
		status = DRAWING_DONE;
	}
	return status;
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
