#include "FreeRTOS.h"
#include "task.h"

#include "lpc40xx.h"
#include "uart_printf.h"

#if ((0 != configCHECK_FOR_STACK_OVERFLOW) || (0 != configUSE_MALLOC_FAILED_HOOK))
static void halt(const char *message) {
  // We may not be able to print this while stack is corrupt but we can try
  uart_puts__polled(UART__0, message);
  while (1) {
  }
}
#endif

#if (0 != configCHECK_FOR_STACK_OVERFLOW)
/// Invoked by FreeRTOS when it detects stack overflow
void vApplicationStackOverflowHook(TaskHandle_t task_handle, signed char *task_name) {
  uart_puts__polled(UART__0, "stack overflow by this task:");
  halt((const char *)task_name);
}
#endif

#if (0 != configUSE_MALLOC_FAILED_HOOK)
/// Invoked by FreeRTOS when there pvPortMalloc() function has encountered NULL pointer to memory request
void vApplicationMallocFailedHook(void) { halt("vApplicationMallocFailedHook() called; no more memory"); }
#endif

#if (0 != configUSE_IDLE_HOOK)
/**
 * FreeRTOS note:
 * vApplicationIdleHook() MUST NOT, UNDER ANY CIRCUMSTANCES, CALL A FUNCTION THAT MIGHT BLOCK
 */
void vApplicationIdleHook(void) {
  LPC_SC->PCON = 0; // Enter sleep mode
  __asm__("WFI");   // Wait for interrupt
}
#endif

#if (0 != configUSE_TICK_HOOK)
/// Called upon each interrupt that invokes the FreeRTOS tick handler
void vApplicationTickHook(void) {}
#endif

#ifdef configASSERT
void configASSERT_callback(unsigned line, const char *message) {
  uart_puts__polled(UART__0, "FreeRTOS ASSERT() occurred indicating an error condition that should NEVER occur");
  halt(message);
}
#endif

#if (0 != configSUPPORT_STATIC_ALLOCATION)
// Copied from https://www.freertos.org/a00110.html
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;            // memory for the idle task
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;        // stack memory for the idle task
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE; // stack memory size
}
#endif
