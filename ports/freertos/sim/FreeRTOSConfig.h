#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
#define configUSE_PREEMPTION 1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configCPU_CLOCK_HZ 100000000u
#define configTICK_RATE_HZ 1000u
#define configMAX_PRIORITIES 8
#define configMINIMAL_STACK_SIZE 2048
#define configTOTAL_HEAP_SIZE (256u * 1024u)
#define configMAX_TASK_NAME_LEN 16
#define configUSE_16_BIT_TICKS 0
#define configUSE_MUTEXES 1
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 1
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY 2
#define configTIMER_QUEUE_LENGTH 8
#define configTIMER_TASK_STACK_DEPTH 2048
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configSUPPORT_STATIC_ALLOCATION 0
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configCHECK_FOR_STACK_OVERFLOW 0
#define configASSERT(x) do { if (!(x)) abort(); } while (0)
#define INCLUDE_vTaskDelay 1
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_xTaskGetIdleTaskHandle 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_xTaskGetTickCount 1
#endif
