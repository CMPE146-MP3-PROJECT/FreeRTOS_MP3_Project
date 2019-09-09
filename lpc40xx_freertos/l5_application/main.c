#include "lpc40xx.h"

#include "delay.h"
#include "gpio.h"
#include "sys_time.h"

#include "FreeRTOS.h"
#include "task.h"

static void blink_task(void *params);
static void blink_on_startup(gpio_s gpio);

static gpio_s led0, led1;

int main(void) {
  sys_time__init(configCPU_CLOCK_HZ);

  led0 = gpio__instantiate(gpio__port_2, 3);
  led1 = gpio__instantiate(gpio__port_1, 26);
  gpio__set_as_output(led0);
  gpio__set_as_output(led1);

  blink_on_startup(led1);

  xTaskCreate((TaskFunction_t)blink_task, "led0", 512U, (void *)&led0, PRIORITY_HIGH, NULL);
  xTaskCreate((TaskFunction_t)blink_task, "led1", 512U, (void *)&led1, PRIORITY_HIGH, NULL);

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

  while (1 == 1) {
    gpio__toggle(gpio);
    vTaskDelay(250);
  }
}

static void blink_on_startup(gpio_s gpio) {
  for (int i = 0; i < 10; i++) {
    delay__ms(250);
    gpio__toggle(gpio);
  }
}
