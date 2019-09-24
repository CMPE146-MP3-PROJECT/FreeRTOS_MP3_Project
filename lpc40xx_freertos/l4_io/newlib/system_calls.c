/**
 * @file
 *
 * This connects the GCC calls to your system, such opening, and writing a file
 * This includes printf() such that it can be glued to our UART0 driver
 * Refer to libc.pdf for more details
 */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "uart.h"

/**
 * Standard values of STDIN, STDOUT, and STDERR
 */
typedef enum {
  file_id__stdin = 0,
  file_id__stdout = 1,
  file_id__stderr = 2,
} file_id_e;

/// printf(), scanf() for standard output uses this UART
static const uart_e system_calls__uart_type = UART__0;

/// Uses uart__put() which deposits data into a queue and returns immediately
static void system_calls__queued_put(const char *ptr, int len) {
  const uint32_t forever_timeout = UINT32_MAX;

  for (int i = 0; i < len; i++) {
    uart__put(system_calls__uart_type, ptr[i], forever_timeout);
  }
}

/// Uses polled version to enforce that the data goes out (the cost is that it eats our CPU)
static void system_calls__polled_put(const char *ptr, int len) {
  for (int i = 0; i < len; i++) {
    uart__polled_put(system_calls__uart_type, ptr[i]);
  }
}

static void system_calls__print_and_halt(const char *string) {
  system_calls__polled_put(string, strlen(string));
  while (1) {
  }
}

/*******************************************************************************
 *
 *                      P U B L I C    F U N C T I O N S
 *
 ******************************************************************************/

/**
 * Handle the kill signal for our platform
 * libc.pdf: 'Send a signal to the process, such as SIGINT (Ctrl+C)'
 */
int _kill(int pid, int sig) {
  system_calls__print_and_halt("ERROR: Unexpected system call to _kill(). Behavior for this system is not defined");
  return 0;
}

/**
 * Handle exit() of the process
 * libc.pdf: 'End a program execution with no cleanup processing'
 * @param code is typically EXIT_SUCCESS or EXIT_FAILURE
 */
void _exit(int code) {
  system_calls__print_and_halt("ERROR: Unexpected system call to _exit(). Behavior for this system is not defined");
  while (1) {
    ; // <unistd.h> _exit() should not return
  }
}

/**
 * libc.pdf: 'Query whether output stream is a terminal'
 */
int _isatty(int file_descriptor) {
  int is_atty = 0;

  switch (file_descriptor) {
  // <unistd.h> defines standard values of stdin, stdout etc.
  case file_id__stdin:
  case file_id__stdout:
  case file_id__stderr:
    is_atty = 1;
    break;

  // Pointer based values defined by <stdio.h>
  default:
    if ((int)stdin == file_descriptor || (int)stdout == file_descriptor || (int)stderr == file_descriptor) {
      is_atty = 1;
    }
  }

  return is_atty;
}

/**
 * Low level _open() used by fopen()
 */
int _open(const char *path, int flags, ...) {
  system_calls__print_and_halt("ERROR: Call to _open() not expected\n");
  return 0;
}

/**
 * Low level write routine used by standard output (printf) and also file writes after fopen() has been called
 */
int _write(int file_descriptor, const char *ptr, int bytes_to_write) {
  if (_isatty(file_descriptor)) {
    const bool is_standard_error = ((int)stderr == file_descriptor || file_id__stderr == file_descriptor);
    const bool rtos_is_running = taskSCHEDULER_RUNNING == xTaskGetSchedulerState();
    const bool transmit_queue_enabled = uart__is_transmit_queue_initialized(system_calls__uart_type);

    /* If the RTOS is running with the UART queue enabled, AND it is not an error printf, then queue
     * the output and return quickly, otherwise there is no choice but to use system_calls__polled_put()
     */
    if (rtos_is_running && transmit_queue_enabled && !is_standard_error) {
      system_calls__queued_put(ptr, bytes_to_write);
    } else {
      system_calls__polled_put(ptr, bytes_to_write);
    }
  } else {
    system_calls__print_and_halt("ERROR: Call to _write() with an unsupported handle");
  }

  return bytes_to_write;
}

/**
 * Low level read routine used by standard input (scanf) and also file reads after fopen() has been called
 */
int _read(int file_descriptor, char *ptr, int len) {
  int bytes_read = 0;

  if (_isatty(file_descriptor)) {
    // This is a little awkward but uart__polled_get() will not poll if RTOS is running with UART RX queue enabled
    if (uart__polled_get(system_calls__uart_type, &ptr[0])) {
      // We deliberately just read 1 char, and let the libc re-invoke _read() again as needed
      ++bytes_read;
    }
  } else {
    system_calls__print_and_halt("ERROR: Call to _read() with an unsupported handle");
  }

  return bytes_read;
}
