#include <stdbool.h>

#include "delay.h"

#include "FreeRTOS.h"
#include "task.h"

#include "sys_time.h"

void delay_ms(uint32_t ms) {
  const uint64_t us = UINT64_C(1000) * ms;
  delay_us(us);
}

void delay_us(uint64_t us) {
  const uint64_t time_now_us = sys_time__get_uptime_us();
  const uint64_t time_to_wait_us = time_now_us + us;

  // Use the RTOS for the larger delay period in milliseconds since this will sleep the CPU
  const bool rtos_is_running = (taskSCHEDULER_RUNNING == xTaskGetSchedulerState());
  if (rtos_is_running && us > 1000) {
    const uint32_t ms = us / 1000;
    vTaskDelay(ms);
  }

  // Eat the CPU for the remaining microseconds
  while (sys_time__get_uptime_us() < time_to_wait_us) { }
}
