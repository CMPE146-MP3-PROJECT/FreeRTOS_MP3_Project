#include "FreeRTOS.h"
#include "task.h"

#include "uart_printf.h"

/**
 * Invoked by FreeRTOS when it detects stack overflow
 */
void vApplicationStackOverflowHook(TaskHandle_t task_handle, signed char *task_name) {
  // We may not be able to print this while stack is corrupt but we can try
  uart_puts__polled(UART__0, (const char *)task_name);

  while (1) {
    ;
  }
}
