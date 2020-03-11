#include "ssp2_mutex.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

static StaticSemaphore_t spi2_mutex_memory;
static SemaphoreHandle_t spi2_mutex_handle;

void spi2_mutex__acquire(void) {
  if (!spi2_mutex_handle) {
    spi2_mutex_handle = xSemaphoreCreateMutexStatic(&spi2_mutex_memory);
    vTraceSetMutexName(spi2_mutex_handle, "spi2_mutex");
  }

  // Only acquire the mutex if the FreeRTOS is running
  if (taskSCHEDULER_RUNNING == xTaskGetSchedulerState()) {
    xSemaphoreTake(spi2_mutex_handle, portMAX_DELAY);
  }
}

void spi2_mutex__release(void) {
  if (taskSCHEDULER_RUNNING == xTaskGetSchedulerState()) {
    xSemaphoreGive(spi2_mutex_handle);
  }
}
