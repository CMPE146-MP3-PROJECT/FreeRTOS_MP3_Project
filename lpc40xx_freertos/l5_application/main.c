#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_io.h"
#include "delay.h"
#include "gpio.h"
#include "uart.h"

static void blink_task(void *params);
static void uart_task(void *params);

static void blink_on_startup(gpio_s gpio, int count);
static void uart0_init(void);

static gpio_s led0, led1;

int main(void) {
  /// UART initialization is required in order to use <stdio.h> puts, printf() etc; @see system_calls.c
  uart0_init();
  puts("\n--------\nStartup");

  // Construct the LEDs and blink a startup sequence
  led0 = board_io__get_led0();
  led1 = board_io__get_led1();
  blink_on_startup(led1, 2);

  puts("Creating tasks");
  xTaskCreate(blink_task, "led0", (512U / sizeof(void *)), (void *)&led0, PRIORITY_LOW, NULL);
  xTaskCreate(blink_task, "led1", (512U / sizeof(void *)), (void *)&led1, PRIORITY_LOW, NULL);

  // printf() takes more stack space
  xTaskCreate(uart_task, "uart", (512U * 8) / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  puts("Starting RTOS");
  vTaskStartScheduler();

  /**
   * vTaskStartScheduler() should never return.
   *
   * Otherwise, it returning indicates there is not enough free memory or scheduler was explicitly terminated
   * CPU will now halt forever at this point.
   */

  return 0;
}

static void blink_task(void *params) {
  const gpio_s led = *((gpio_s *)params);

  while (true) {
    gpio__toggle(led);
    vTaskDelay(500);
  }
}

static void uart_task(void *params) {
  TickType_t previous_tick = 0;
  long ticks = 0;

  while (true) {
    // This loop will repeat every 500 ticks, even if the logic below takes variable amount of ticks
    vTaskDelayUntil(&previous_tick, 500);

    /* Calls to fprintf(stderr, ...) uses polled UART driver, so this entire output will be fully sent out
     * before this function returns. See system_calls.c for actual implementation.
     * This is useful to print information inside of interrupts as you cannot use printf() inside an ISR
     */
    ticks = xTaskGetTickCount();
    fprintf(stderr, "This is a polled version of the printf using for debugging ... finished in");
    fprintf(stderr, " %lu ticks\n", (xTaskGetTickCount() - ticks));

    /* This deposits data to an outgoing queue and doesn't block the CPU
     * Data will be sent later, but this function would return earlier
     */
    ticks = xTaskGetTickCount();
    printf("This is a more efficient printf ... finished in");
    printf(" %lu ticks\n", (xTaskGetTickCount() - ticks));
  }
}

static void blink_on_startup(gpio_s gpio, int blinks) {
  const int toggles = (2 * blinks);
  for (int i = 0; i < toggles; i++) {
    delay__ms(250);
    gpio__toggle(gpio);
  }
}

static void uart0_init(void) {
  // Note: PIN functions are initialized by board_io__initialize() for P0.2(Tx) and P0.3(Rx)
  uart__init(UART__0, clock__get_peripheral_clock_hz(), 115200);

  // Make UART more efficient by backing it with RTOS queues (optional but highly recommended with RTOS)
  QueueHandle_t tx_queue = xQueueCreate(128, sizeof(char));
  QueueHandle_t rx_queue = xQueueCreate(32, sizeof(char));
  uart__enable_queues(UART__0, tx_queue, rx_queue);
}
