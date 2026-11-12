#ifndef ER_OSAL_FREERTOS_H
#define ER_OSAL_FREERTOS_H
#include "er/osal.h"
#include "FreeRTOS.h"
#include "task.h"

/* The application owns the task. This port supplies the clock contract only;
 * it does not create tasks, queues, or mutexes. Tick resolution must be
 * selected and measured for the product. */
er_osal_clock_t er_osal_freertos_clock(void);
#endif
