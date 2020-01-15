#include "FreeRTOS.h"
#include "task.h"

#include "lpc40xx.h"
#include "sys_time.h"
#include "uart_printf.h"

/**
 * Some of the comments indicate when we need the function, such as '#if (0 != configCHECK_FOR_STACK_OVERFLOW)'
 * Instead of plaguing the code with a bunch of #ifdef, we let the link time optimizer remove the function
 * from the compiled binary
 */

static uint64_t freertos_run_time_counter_offset;

static void halt(const char *message) {
  // We may not be able to print this while stack is corrupt but we can try
  uart_puts__polled(UART__0, message);
  while (1) {
  }
}

/// Invoked by FreeRTOS when it detects stack overflow: #if (0 != configCHECK_FOR_STACK_OVERFLOW)
void vApplicationStackOverflowHook(TaskHandle_t task_handle, signed char *task_name) {
  uart_puts__polled(UART__0, "stack overflow by this task:");
  halt((const char *)task_name);
}

// Invoked by FreeRTOS when there pvPortMalloc() function has encountered NULL pointer to memory request
// #if (0 != configUSE_MALLOC_FAILED_HOOK)
void vApplicationMallocFailedHook(void) { halt("vApplicationMallocFailedHook() called; no more RAM memory"); }

/**
 * FreeRTOS note:
 * vApplicationIdleHook() MUST NOT, UNDER ANY CIRCUMSTANCES, CALL A FUNCTION THAT MIGHT BLOCK
 * #if (0 != configUSE_IDLE_HOOK)
 */
void vApplicationIdleHook(void) {
  LPC_SC->PCON = 0; // Enter sleep mode
  __asm__("WFI");   // Wait for interrupt
}

/// Called upon each interrupt that invokes the FreeRTOS tick handler: #if (0 != configUSE_TICK_HOOK)
void vApplicationTickHook(void) {
  ; // This code is invoked during each RTOS tick interrupt
}

// This is needed when we use '#ifdef configASSERT'
void configASSERT_callback(unsigned line, const char *message) {
  uart_puts__polled(UART__0, "FreeRTOS ASSERT() occurred indicating an error condition that should NEVER happen");
  uart_puts__polled(UART__0, " - Did you call a blocking API or non FromISR() API inside an ISR?");
  uart_puts__polled(UART__0, " - Did you forget to use fprintf(stderr) in an ISR?");
  uart_puts__polled(UART__0, "Here is the line of code that halted the CPU: ");
  halt(message);
}

// Copied from https://www.freertos.org/a00110.html, needed when #if (0 != configSUPPORT_STATIC_ALLOCATION)
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;            // memory for the idle task
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;        // stack memory for the idle task
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE; // stack memory size
}

// #if (0 != configGENERATE_RUN_TIME_STATS)
uint32_t freertos_hooks__get_run_time_counter_value(void) {
  return (uint32_t)(sys_time__get_uptime_us() - freertos_run_time_counter_offset);
}

void freertos_hooks__reset_run_time_stats(void) {
  // We save the offset that acts like a way to reset the uint64_t counters
  freertos_run_time_counter_offset = sys_time__get_uptime_us();
}