/*
 * Copyright 2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include <string.h>
#include <stdbool.h>
#include "LLEXT_RES_impl.h"
#include "sni.h"

/* Defines -------------------------------------------------------------------*/

#define PATH_BUFFER_SIZE 100

/* Types ---------------------------------------------------------------------*/

typedef struct {
	void* data;
	uint32_t size;
} SNIX_resource;

/* Extern --------------------------------------------------------------------*/

extern int32_t SNIX_get_resource(char* path, SNIX_resource* resource);

/* Globals -------------------------------------------------------------------*/
// cppcheck-suppress [misra-c2012-8.9]: Global buffer needed for the java world.
static char g_path_buffer[PATH_BUFFER_SIZE];
static SNIX_resource g_current_resource;

/* Private functions ---------------------------------------------------------*/

static char* convert_path(char* path, uint32_t offset, size_t* path_length)
{
	char* ret = NULL;
	// check if g_path_buffer can contain given path
	*path_length = strlen(path) + (size_t)1 /* /0 */;
	if (*path_length < PATH_BUFFER_SIZE)
	{
		// copy path to g_path_buffer
		(void) strncpy(g_path_buffer + offset, path, *path_length);

		// replace path separator by a '_' (prevent to use directory)
		char* local_path = (char*)g_path_buffer;
		if (local_path[offset] == 'j'){
			local_path[offset] = 'i';
		}
		ret = g_path_buffer;
	}

	return ret;
}

/* API -----------------------------------------------------------------------*/

/**
 * Open the resource whose name is the string pointed to by path.
 * @param path a null terminated string.
 * @return the resource ID on success, a negative value on error.
 */
RES_ID LLEXT_RES_open(const char* path)
{	
	//Debug_Printf("%s %s\n", __FUNCTION__, path);

	size_t path_length;
	RES_ID ret = 0;
	// cppcheck-suppress [misra-c2012-11.8] Need to cast for the function signature.
	char* fs_path = convert_path((char*)path, 1, &path_length);
	fs_path[0] ='/';
	// look for resource
	ret = SNIX_get_resource(fs_path, &g_current_resource);
	if (ret != 0)
	{
		ret = (RES_ID)-1;
	}

	return (RES_ID)ret;
}

/**
 * Close the resource referenced by the given resourceID
 * @param resourceID an ID returned by the LLEXT_RES_open function.
 * @return LLEXT_RES_OK on success, a negative value on error.
 */
int32_t LLEXT_RES_close(RES_ID resourceID)
{
	(void) resourceID;
	return LLEXT_RES_OK;
}

/**
 * Returns the resource base address or -1 when not available.
 *
 * The resource base address is an address available in CPU address space range. That means the
 * CPU can access to the resource memory using the same assembler instructions than the CPU internal memories.
 * In this case this memory is not considered as external memory. This address will be used by
 * the caller even if the resource is closed.
 *
 * If the memory is outside the CPU address space range, this function should returns -1. In this
 * case the caller will use this memory as external memory and will have to perform some memory copies in RAM.
 * The memory accesses may be too slow. To force the caller to consider this memory as an external
 * memory whereas this memory is available in CPU address space range, this function should returns -1 too.
 *
 * The returned address is always the resource base address. This address must not change during application
 * execution.
 *
 * @param resourceID an ID returned by the openResource method.
 * @return resource address or -1 when address is outside CPU address space range.
 */
int32_t LLEXT_RES_getBaseAddress(RES_ID resourceID)
{
	(void) resourceID;
	return -1; //(int32_t)(g_current_resource.data) ;
}

/**
 * Try to read size bytes from the stream referenced by the given resourceID.
 * The read bytes are copied at given ptr. If given ptr is <code>NULL</code>
 * then size bytes are skipped from the stream. Parameter <code>size</code>
 * is a pointer on an integer which defines the maximum number of bytes to
 * read or skip. This value must be updated by the number of bytes read or
 * skipped.
 * @param resourceID an ID returned by the LLEXT_RES_open function.
 * @param ptr the buffer into which the data is read. <code>NULL</code> to skip data.
 * @param size the maximum number of bytes to read or skip.
 * @return LLEXT_RES_OK on success, LLEXT_RES_EOF when end of file is reached, another negative value on error.
 */
int32_t LLEXT_RES_read(RES_ID resourceID, void* ptr, int32_t* size)
{
	(void) resourceID;
	(void) memcpy(ptr, (void*)(g_current_resource.data), *size);
	return LLEXT_RES_OK;
}

/**
 * Returns an estimate of the number of bytes that can be read from the stream referenced by the given resourceID without blocking by the next read.
 * @param resourceID an ID returned by the LLEXT_RES_open function.
 * @return an estimate of the number of bytes that can be read, 0 when end of file is reached, a negative value on error.
 */
int32_t LLEXT_RES_available(RES_ID resourceID)
{	
	(void) resourceID;
	return g_current_resource.size;
}

/**
 * Sets the file position indicator for the stream pointed to by resourceID.
 * The new position, measured in bytes, is obtained by adding offset bytes to the start of the file.
 * @param resourceID an ID returned by the LLEXT_RES_open function.
 * @param offset new value in bytes of the file position indicator.
 * @return LLEXT_RES_OK on success, a negative value on error.
 */
int32_t LLEXT_RES_seek(RES_ID resourceID, int64_t offset)
{
	(void) resourceID;
	(void) offset;
	return -1;
}

/**
 * Obtains the current value of the file position indicator for the stream pointed to by resourceID.
 * @param resourceID an ID returned by the LLEXT_RES_open function.
 * @return the current value in bytes of the file position indicator on success, a negative value on error.
 */
int64_t LLEXT_RES_tell(RES_ID resourceID)
{	
	(void) resourceID;
	return -1;
}

