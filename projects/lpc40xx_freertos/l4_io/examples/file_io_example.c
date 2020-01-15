#include <stdbool.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "ff.h"
#include "sys_time.h"

/**
 * This is a file reader task that will continously open a file and read all its contents
 * It can be run as a test task by adding this sample code where "file.txt" is the filename
 *
    void file_io_example__task(void *params);
    xTaskCreate(file_io_example__task, "file", (512U * 8) / sizeof(void *), "file.txt", PRIORITY_HIGH, NULL);
 */
void file_io_example__task(void *params) {
  const char *filename = (const char *)params;
  FIL file;
  long ticks = 0;

  while (true) {
    FRESULT result = f_open(&file, filename, FA_READ);

    if (FR_OK == result) {
      ticks = xTaskGetTickCount();
      UINT br;
      static char buffer[4 * 1024];
      while (FR_OK == f_read(&file, buffer, sizeof(buffer), &br)) {
        if (f_eof(&file)) {
          break;
        }
      }
      f_close(&file);
    }

    if (FR_OK == result) {
      printf("%lu: File read successfully: %lu bytes in %lu ticks\n", xTaskGetTickCount(), f_size(&file),
             (xTaskGetTickCount() - ticks));
    } else {
      printf("File failed to be opened, error %i\n", result);
    }

    vTaskDelay(1000);
  }
}