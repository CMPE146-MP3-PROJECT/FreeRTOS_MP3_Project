#include "ssp2_mutex.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

static SemaphoreHandle_t spi2_mutex;

void spi2_mutex__acquire(void) {
  if (!spi2_mutex) {
    spi2_mutex = xSemaphoreCreateMutex();
  }

  // Only acquire the mutex if the FreeRTOS is running
  if (taskSCHEDULER_RUNNING == xTaskGetSchedulerState()) {
    xSemaphoreTake(spi2_mutex, portMAX_DELAY);
  }
}

void spi2_mutex__release(void) {
  if (taskSCHEDULER_RUNNING == xTaskGetSchedulerState()) {
    xSemaphoreGive(spi2_mutex);
  }
}
