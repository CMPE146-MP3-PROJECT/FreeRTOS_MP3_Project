#pragma once

#include <stdarg.h>

#include "uart.h"

/**
 * Uses the uart__put() API to print out each character
 * internally uses sprintf() to print on a fixed buffer before printing the stream to the UART
 */
int uart_printf(uart_e uart, const char *format, ...) __attribute__((format(printf, 2, 3)));

void uart_puts(uart_e uart, const char *message);

/**
 * Similar to uart_printf() but uses uart__polled_put()
 * This is helpful during debugging if you want to make sure your data is printed entirely before this function exits
 */
int uart_printf__polled(uart_e uart, const char *format, ...) __attribute__((format(printf, 2, 3)));

void uart_puts__polled(uart_e uart, const char *message);