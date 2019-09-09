#include "lpc40xx.h"

#include "FreeRTOS.h"
#include "task.h"

static void blink_task(void *params);

int main(void) {
  void *led1 = (void *)(1 << 26);
  void *led2 = (void *)(1 << 3);

  xTaskCreate((TaskFunction_t)blink_task, "task", 512U, led1, PRIORITY_HIGH, NULL);
  xTaskCreate((TaskFunction_t)blink_task, "task", 512U, led2, PRIORITY_HIGH, NULL);
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
  const unsigned pin_number = (unsigned)params;
  LPC_GPIO1->DIR |= pin_number;

  while (1 == 1) {
    LPC_GPIO1->PIN ^= pin_number;
    vTaskDelay(1000U);
  }
}
