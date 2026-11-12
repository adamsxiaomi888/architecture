#ifndef ER_CONTROL_TASK_H
#define ER_CONTROL_TASK_H
#include "er/osal.h"
/* Same control scenario is callable from POSIX, FreeRTOS or another task
 * entry. The caller supplies only the OSAL clock; no OS headers are needed. */
int er_control_task_run(const er_osal_clock_t *clock, const char *scenario);
#endif
