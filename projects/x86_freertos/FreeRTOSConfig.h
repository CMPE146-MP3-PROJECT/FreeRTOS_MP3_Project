#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION                              1
#define configUSE_IDLE_HOOK                               1
#define configUSE_TICK_HOOK                               0
#define configTICK_RATE_HZ                                ((portTickType)1000)

#define configUSE_MALLOC_FAILED_HOOK                      1

// Not used for FreeRTOS POSIX based simulator
#define configMINIMAL_STACK_SIZE                          ((unsigned portSHORT)64)

// Do not use on FreeRTOS POSIX based simulator
#define configCHECK_FOR_STACK_OVERFLOW                    0
#define INCLUDE_uxTaskGetStackHighWaterMark               0
//#define configMAX_SYSCALL_INTERRUPT_PRIORITY            0

// Only used based on memory management scheme; see SConscript file
#define configTOTAL_HEAP_SIZE                             ((size_t)(64 * 1024))

#define configMAX_TASK_NAME_LEN                           (16)
#define configUSE_TRACE_FACILITY                          1
#define configUSE_16_BIT_TICKS                            0
#define configIDLE_SHOULD_YIELD                           1
#define configUSE_MUTEXES                                 1
#define configUSE_RECURSIVE_MUTEXES                       1
#define configQUEUE_REGISTRY_SIZE                         20
#define configUSE_APPLICATION_TASK_TAG                    1
#define configUSE_COUNTING_SEMAPHORES                     1
#define configUSE_ALTERNATIVE_API                         0

#define configUSE_QUEUE_SETS                              1
#define configUSE_TASK_NOTIFICATIONS                      1

/* Software timer related configuration options. */
#define configUSE_TIMERS                                  1
#define configTIMER_TASK_PRIORITY                         (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                          20
#define configTIMER_TASK_STACK_DEPTH                      (configMINIMAL_STACK_SIZE * 2)

/**
 * Task Priorities
 * Priority 0 is for IDLE task and should not be used
 */
#define PRIORITY_LOW                                      1
#define PRIORITY_MEDIUM                                   2
#define PRIORITY_HIGH                                     3
#define PRIORITY_CRITICAL                                 4
#define configMAX_PRIORITIES                              (1 + PRIORITY_CRITICAL)

#define configGENERATE_RUN_TIME_STATS                     1

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function.  In most cases the linker will remove unused
functions anyway. */
#define INCLUDE_vTaskPrioritySet                          1
#define INCLUDE_uxTaskPriorityGet                         1
#define INCLUDE_vTaskDelete                               1
#define INCLUDE_vTaskSuspend                              1
#define INCLUDE_vTaskDelayUntil                           1
#define INCLUDE_vTaskDelay                                1
#define INCLUDE_xTaskGetSchedulerState                    1
#define INCLUDE_xTimerGetTimerDaemonTaskHandle            1
#define INCLUDE_xTaskGetIdleTaskHandle                    1
#define INCLUDE_pcTaskGetTaskName                         1
#define INCLUDE_eTaskGetState                             1
#define INCLUDE_xSemaphoreGetMutexHolder                  1
#define INCLUDE_xTimerPendFunctionCall                    1
#define INCLUDE_xTaskAbortDelay                           1
#define INCLUDE_xTaskGetHandle                            1

/* It is a good idea to define configASSERT() while developing.  configASSERT()
uses the same semantics as the standard C assert() macro. */
extern void vAssertCalled(unsigned long ulLine, const char *const pcFileName);
#define configASSERT(x)                                                                                                \
  if ((x) == 0)                                                                                                        \
  vAssertCalled(__LINE__, __FILE__)

// If RTOS trace facility is enabled, then provide the high resolution timer API
#if (0 != configGENERATE_RUN_TIME_STATS)
  // These are defined by x86 portmacro
  //#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
  //#define portGET_RUN_TIME_COUNTER_VALUE()          
  #define portRESET_TIMER_FOR_RUN_TIME_STATS()      
#endif

#endif /* FREERTOS_CONFIG_H */
