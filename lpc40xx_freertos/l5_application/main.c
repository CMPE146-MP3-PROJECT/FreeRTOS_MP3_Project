#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

#include "delay.h"
#include "gpio.h"
#include "uart.h"
#include "uart_printf.h"

static void blink_task(void *params);
static void uart_task(void *params);

static void blink_on_startup(gpio_s gpio);
static void uart0_init(void);

static gpio_s led0, led1;

int main(void) {
  uart0_init();

  // Construct the LEDs and blink a startup sequence
  led0 = gpio__construct_as_output(gpio__port_2, 3);
  led1 = gpio__construct_as_output(gpio__port_1, 26);
  blink_on_startup(led1);

  xTaskCreate(blink_task, "led0", (512U / sizeof(void *)), (void *)&led0, PRIORITY_LOW, NULL);
  xTaskCreate(blink_task, "led1", (512U / sizeof(void *)), (void *)&led1, PRIORITY_LOW, NULL);

  // printf() takes more stack space
  xTaskCreate(uart_task, "uart", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);

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
  while (true) {
    vTaskDelay(500);

    // Wait until the data is fully printed before moving on
    uart_printf__polled(UART__0, "Hello world\n");

    // This deposits data to an outgoing queue and doesn't block the CPU
    uart_printf(UART__0, " ... and a more efficient printf...\n");
  }
}

static void blink_on_startup(gpio_s gpio) {
  for (int i = 0; i < 10; i++) {
    delay__ms(100);
    gpio__toggle(gpio);
  }
}

static void uart0_init(void) {
  (void)gpio__construct_with_function(gpio__port_0, 2, gpio__function_1); // P0.2 - Uart-0 Tx
  (void)gpio__construct_with_function(gpio__port_0, 3, gpio__function_1); // P0.3 - Uart-0 Rx

  uart__init(UART__0, clock__get_core_clock_hz(), 115200);

  // Make UART more efficient by backing it with RTOS queues (optional but highly recommended with RTOS)
  QueueHandle_t tx_queue = xQueueCreate(128, sizeof(char));
  QueueHandle_t rx_queue = xQueueCreate(32, sizeof(char));
  uart__enable_queues(UART__0, tx_queue, rx_queue);
}
