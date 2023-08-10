/*
 * C
 *
 * Copyright 2018-2021 MicroEJ Corp. All rights reserved.
 * This library is provided in source code for use, modification and test, subject to license terms.
 * Any modification of the source code will break MicroEJ Corp. warranties on the whole library.
 */

/**
* @file
* @brief MicroEJ Trace library low level API
* @author MicroEJ Developer Team
* @version 2.1.2
* @date 29 September 2021
*/

#ifndef LLMJVM_MONITOR_SYSVIEW_H
#define LLMJVM_MONITOR_SYSVIEW_H

#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif

/*
 * This function is part of the link between MicroEJ and SYSVIEW.
 * Called from SystemView when asked by the host, it uses SYSVIEW
 * functions to send the entire task list to the host.
 * It is typically set to <code>pfSendTaskList</code> of the <code>SYSVIEW_X_OS_TraceAPI</code> structure.
};
 */
void LLMJVM_MONITOR_SYSTEMVIEW_send_task_list(void);

#ifdef __cplusplus
	}
#endif
#endif //LLMJVM_MONITOR_SYSVIEW_H
