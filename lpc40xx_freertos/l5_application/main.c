#include <stdbool.h>

#include "lpc40xx.h"

#include "FreeRTOS.h"
#include "task.h"

#include "delay.h"
#include "gpio.h"
#include "sys_time.h"
#include "clock.h"

#include "uart.h"
#include "uart_printf.h"

static void blink_task(void *params);
static void uart_task(void *params);

static void blink_on_startup(gpio_s gpio);
static void uart0_init(void);

static gpio_s led0, led1;

int main(void) {
  sys_time__init(clock__get_core_clock_hz());

  led0 = gpio__instantiate(gpio__port_2, 3);
  led1 = gpio__instantiate(gpio__port_1, 26);
  gpio__set_as_output(led0);
  gpio__set_as_output(led1);

  uart0_init();
  blink_on_startup(led1);

  xTaskCreate(blink_task, "led0", 512U, (void *)&led0, PRIORITY_LOW, NULL);
  xTaskCreate(blink_task, "led1", 512U, (void *)&led1, PRIORITY_LOW, NULL);
  xTaskCreate(uart_task, "uart", 512U, NULL, PRIORITY_LOW, NULL);

  vTaskStartScheduler();

  /**
   * vTaskStartScheduler() should never return.
   *
   * Otherwise, it returning indicates there is not enough free memory or scheduler was explicitly terminated
   * CPU will now halt forever at this point.
   */
  while (1) {
  }

  return -1;
}

static void blink_task(void *params) {
  const gpio_s gpio = *((gpio_s *)params);

  while (true) {
    gpio__toggle(gpio);
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
  const gpio_s u0_txd = gpio__instantiate(gpio__port_0, 2); // P0.2
  const gpio_s u0_rxd = gpio__instantiate(gpio__port_0, 3); // P0.3

  gpio__set_function(u0_txd, gpio__function_1);
  gpio__set_function(u0_rxd, gpio__function_1);

  uart__init(UART__0, clock__get_core_clock_hz(), 115200);

  // Make UART more efficient by backing it with RTOS queues (optional but highly recommended)
  QueueHandle_t tx_queue = xQueueCreate(128, sizeof(char));
  QueueHandle_t rx_queue = xQueueCreate(32, sizeof(char));
  uart__enable_queues(UART__0, tx_queue, rx_queue);
}
