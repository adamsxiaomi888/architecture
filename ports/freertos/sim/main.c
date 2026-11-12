#include "FreeRTOS.h"
#include "task.h"
#include "er/osal.h"
#include "control_task.h"
#include "osal_freertos.h"
#include <stdio.h>
#include <stdlib.h>

static void control_task(void *arg)
{
    (void)arg;
    er_osal_clock_t clock = er_osal_freertos_clock();
    int rc = er_control_task_run(&clock, "normal");
    printf("freertos_sim control_task rc=%d\n", rc);
    configASSERT(rc == 0);
    vTaskEndScheduler();
    vTaskDelete(NULL);
}

int main(void)
{
    if (xTaskCreate(control_task, "control", 4096, NULL,
                    configMAX_PRIORITIES - 1, NULL) != pdPASS) return 1;
    vTaskStartScheduler();
    return 0;
}
