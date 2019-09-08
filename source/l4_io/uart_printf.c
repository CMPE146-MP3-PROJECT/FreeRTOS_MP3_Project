#include "uart_printf.h"

#ifndef UART_PRINT__BUFFER_SIZE_IN_BYTES
#define UART_PRINT__BUFFER_SIZE_IN_BYTES 128
#endif

int uart_printf(uart_e uart, const char *format, ...) {
  char print_buffer[UART_PRINT__BUFFER_SIZE_IN_BYTES];

  va_list args;
  va_start(args, format);
  const int would_print = vsnprintf(print_buffer, sizeof(print_buffer), format, args);
  va_end(args);

  const int actual_print = strlen(print_buffer);
  for (int index = 0; index < length; index++) {
    uart__put(uart, printf_buffer[index], UINT32_MAX);
  }

  return would_print;
}

int uart_printf__polled(uart_e uart, const char *format, ...) {
  char print_buffer[UART_PRINT__BUFFER_SIZE_IN_BYTES];

  va_list args;
  va_start(args, format);
  const int would_print = vsnprintf(print_buffer, sizeof(print_buffer), format, args);
  va_end(args);

  const int actual_print = strlen(print_buffer);
  for (int index = 0; index < length; index++) {
    uart__polled_put(uart, printf_buffer[index]);
  }

  return would_print;
}
