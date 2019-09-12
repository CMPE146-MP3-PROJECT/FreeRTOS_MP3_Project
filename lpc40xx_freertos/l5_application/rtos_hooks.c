#include "FreeRTOS.h"
#include "task.h"

#include "uart_printf.h"

#if ((configCHECK_FOR_STACK_OVERFLOW != 0) || (configUSE_MALLOC_FAILED_HOOK == 1))
static void halt(const char *message) {
  // We may not be able to print this while stack is corrupt but we can try
  uart_puts__polled(UART__0, message);
  while (1) {
  }
}
#endif

#if (configCHECK_FOR_STACK_OVERFLOW != 0)
/// Invoked by FreeRTOS when it detects stack overflow
void vApplicationStackOverflowHook(TaskHandle_t task_handle, signed char *task_name) { halt((const char *)task_name); }
#endif

#if (configUSE_MALLOC_FAILED_HOOK == 1)
/// Invoked by FreeRTOS when there pvPortMalloc() function has encountered NULL pointer to memory request
void vApplicationMallocFailedHook(void) { halt("vApplicationMallocFailedHook() called; no more memory"); }
#endif

#if (configUSE_IDLE_HOOK == 1)
/**
 * FreeRTOS note:
 * vApplicationIdleHook() MUST NOT, UNDER ANY CIRCUMSTANCES, CALL A FUNCTION THAT MIGHT BLOCK
 */
void vApplicationIdleHook(void) {
  // TODO: Put the CPU to sleep
}
#endif

#if (configUSE_TICK_HOOK == 1)
/// Called upon each interrupt that invokes the FreeRTOS tick handler
void vApplicationTickHook(void) {}
#endif