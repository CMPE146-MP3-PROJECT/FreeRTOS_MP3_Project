#pragma once

#include "clock.h"

/**
 * Task Priorities
 * Priority 0 is for IDLE task and should not be used
 */
#define PRIORITY_LOW                            1
#define PRIORITY_MEDIUM                         2
#define PRIORITY_HIGH                           3
#define PRIORITY_CRITICAL                       4
#define configMAX_PRIORITIES                    (1 + PRIORITY_CRITICAL)

/**
 * port.c sets up CPU HW timer interrupt to occur at configTICK_RATE_HZ
 * to drive the preemptive scheduler
 */
#define configCPU_CLOCK_HZ                      (clock__get_core_clock_hz())
#define configTICK_RATE_HZ                      ((TickType_t) 1000)
#define configUSE_PREEMPTION                    1

#define configMINIMAL_STACK_SIZE                ((unsigned short) 512 / sizeof(void*))
#define configCHECK_FOR_STACK_OVERFLOW          (2)

#define configMAX_TASK_NAME_LEN                 (12)
#define configIDLE_SHOULD_YIELD                 1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    255
#define configKERNEL_INTERRUPT_PRIORITY         255

#define configUSE_MUTEXES                       1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_QUEUE_SETS                    1

#define configUSE_TIMERS                        0
#define configTIMER_TASK_PRIORITY               (PRIORITY_HIGH)
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            (configMINIMAL_STACK_SIZE * 2)

#define configUSE_IDLE_HOOK                     1
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            1
#define configUSE_16_BIT_TICKS                  0

#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_xTaskGetSchedulerState          1

#define INCLUDE_xTaskGetHandle                  1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_eTaskGetState                   1

#if 1
#define configASSERT(condition) \
do {                 \
  if(!(condition)) { \
    while(1) {}      \
  }                  \
} while(0)
#endif

/**
 * Sanity checks
 */
#if (configMAX_PRIORITIES <= PRIORITY_CRITICAL)
  #error "Max priority must be greater than CRITICAL!"
#endif
