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

/**
 * Priorities used by the periodic_scheduler.h
 * Your tasks should not use these priorities as the 'periodic scheduler' that
 * invokes the callbacks at their respective time are meant to be super precise
 * callbacks with higher priorities than the priorities defined above (1-4)
 */
#define PRIORITY_PERIODIC_1HZ                   5
#define PRIORITY_PERIODIC_10HZ                  6
#define PRIORITY_PERIODIC_100HZ                 7
#define PRIORITY_PERIODIC_1000HZ                8
#define PRIORITY_PERIODIC_MONITOR               9

#define configMAX_PRIORITIES                    (1 + PRIORITY_PERIODIC_MONITOR) // +1 for idle task

/**
 * port.c sets up CPU HW timer interrupt to occur at configTICK_RATE_HZ
 * to drive the preemptive scheduler
 */
#define configCPU_CLOCK_HZ                      (clock__get_core_clock_hz())
#define configTICK_RATE_HZ                      ((TickType_t) 1000)
#define configUSE_PREEMPTION                    1
#define RTOS_MS_TO_TICKS(milliseconds)          (milliseconds == UINT32_MAX) ? (portMAX_DELAY) : (((uint64_t)configTICK_RATE_HZ * milliseconds) / 1000)

/**
 * Size of stack does not need alignment on ARM Cortex
 * This minimal stack is good enough for basic tasks; be careful not to use printf() with this minimal stack size
 */
#define configMINIMAL_STACK_SIZE                (400U / sizeof(void*))
#define configCHECK_FOR_STACK_OVERFLOW          (2)

#define configMAX_TASK_NAME_LEN                 (12)
#define configIDLE_SHOULD_YIELD                 1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         1

/**
 * @{
 * @name Important priority configuration for Cortex M
 * @see http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html
 */
#include "lpc40xx.h"
#ifndef __NVIC_PRIO_BITS
  #error "__NVIC_PRIO_BITS must be defined"
#else
  #define configPRIO_BITS   __NVIC_PRIO_BITS
  #if (5 != __NVIC_PRIO_BITS)
    #error "Unexpected value of __NVIC_PRIO_BITS, lpc40xx has 5-bit priorities (32 levels)"
  #endif
#endif

/**
 * All interrupts of higher priority (or lower number) must not use RTOS API
 * All interrupts of lower or equal priority (higher number) can use FreeRTOS FromISR() API
 *
 * Based on the value of 3, we can have priority 1, and priority 2 nest, which means these
 * interrupts are higher than RTOS interrupts
 */
#define RTOS_HIGHEST_INTERRUPT_PRIORITY         (0x03) /* Your interrupts must be equal or lower priority (higher number) than this */
#define configPRIO_BITS_NOT_IMPLEMENTED         (8 - configPRIO_BITS) /* Number of un-implemented priority bits */

#define configKERNEL_INTERRUPT_PRIORITY         (0x1F << configPRIO_BITS_NOT_IMPLEMENTED)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (RTOS_HIGHEST_INTERRUPT_PRIORITY << configPRIO_BITS_NOT_IMPLEMENTED)
/** @} */

#define configUSE_MUTEXES                       1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_QUEUE_SETS                    0

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

// Enable CPU utilization API
#define configUSE_TRACE_FACILITY                1
#define configGENERATE_RUN_TIME_STATS           1

// If RTOS trace facility is enabled, then provide the high resolution timer API
#if (0 != configUSE_TRACE_FACILITY)
  extern uint32_t freertos_hooks__get_run_time_counter_value(void);
  extern void freertos_hooks__reset_run_time_stats(void);

  #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()  /* sys_init() is initialized before FreeRTOS starts */
  #define portGET_RUN_TIME_COUNTER_VALUE()          freertos_hooks__get_run_time_counter_value()
  #define portRESET_TIMER_FOR_RUN_TIME_STATS()      freertos_hooks__reset_run_time_stats()
#endif

/**
 * When FreeRTOS runs into an assertion that should never occur
 * You can change #if 0 to #if 1 for production, but NEVER change it during development
 */
#if 1
void configASSERT_callback(unsigned line, const char *message);

#define configASSERT(condition)                   \
do {                                              \
  if(!(condition)) {                              \
    configASSERT_callback(__LINE__, #condition);  \
    while(1) {}                                   \
  }                                               \
} while(0)
#endif

/**
 * Sanity checks
 */
#if (configMAX_PRIORITIES <= PRIORITY_CRITICAL)
  #error "Max priority must be greater than CRITICAL!"
#endif

/**
 * Priority 0 (highest priority) interrupt cannot be masked
 * SYSCALL interrupt priority needs to be higher than that
 */
#if (0 == configMAX_SYSCALL_INTERRUPT_PRIORITY)
  #error "configMAX_SYSCALL_INTERRUPT_PRIORITY must not be 0"
#endif
