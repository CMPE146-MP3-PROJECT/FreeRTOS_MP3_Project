#include <stdio.h>
#include <time.h>

// Defined at FreeRTOSConfig.h
void vAssertCalled(unsigned long ulLine, const char *const pcFileName) {
  printf("CRITICAL RTOS ASSERT: %s : %lu\n", pcFileName, ulLine);
}

void vApplicationMallocFailedHook(void) {
  // vApplicationMallocFailedHook() will only be called if configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.

  /*
  It is a hook function that will get called if a call to pvPortMalloc() fails.
  pvPortMalloc() is called internally by the kernel whenever a task, queue,
  timer or semaphore is created.  It is also called by various parts of the
  demo application.  If heap_1.c or heap_2.c are used, then the size of the
  heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
  FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
  to query the size of free heap space that remains (although it does not
  provide information on how the remaining heap might be fragmented).
  */

  vAssertCalled(__LINE__, __FILE__);
}

void vApplicationIdleHook(void) {
  // vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set to 1 in FreeRTOSConfig.h

  /*
  It will be called on each iteration of the idle task.
  It is essential that code added to this hook function never attempts
  to block in any way (for example, call xQueueReceive() with a block time
  specified, or call vTaskDelay()).  If the application makes use of the
  vTaskDelete() API function (as this demo application does) then it is also
  important that vApplicationIdleHook() is permitted to return to its calling
  function, because it is the responsibility of the idle task to clean up
  memory allocated by the kernel to any task that has since been deleted.
  */

  // We should sleep here for a few hundred nanoseconds otherwise this
  // FreeRTOS POSIX simulator will consume 100% host CPU core
  nanosleep((const struct timespec[]){{0, 100L * 1000}}, NULL);
}
