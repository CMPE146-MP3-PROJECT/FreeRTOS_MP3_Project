#include <stdio.h>
#include <string.h>

#include "uart_printf.h"

// uart_printf API uses this sized stack buffer to print messages; this can be re-defined if you need to override it
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
  for (int index = 0; index < actual_print; index++) {
    uart__put(uart, print_buffer[index], UINT32_MAX);
  }

  return would_print;
}

void uart_puts(uart_e uart, const char *message) {
  const size_t message_length = strlen(message);
  for (size_t byte = 0; byte < message_length; byte++) {
    uart__put(uart, message[byte], UINT32_MAX);
  }
  uart__put(uart, '\n', UINT32_MAX);
}

int uart_printf__polled(uart_e uart, const char *format, ...) {
  char print_buffer[UART_PRINT__BUFFER_SIZE_IN_BYTES];

  va_list args;
  va_start(args, format);
  const int would_print = vsnprintf(print_buffer, sizeof(print_buffer), format, args);
  va_end(args);

  const int actual_print = strlen(print_buffer);
  for (int index = 0; index < actual_print; index++) {
    uart__polled_put(uart, print_buffer[index]);
  }

  return would_print;
}

void uart_puts__polled(uart_e uart, const char *message) {
  const size_t message_length = strlen(message);
  for (size_t byte = 0; byte < message_length; byte++) {
    uart__polled_put(uart, message[byte]);
  }
  uart__polled_put(uart, '\n');
}