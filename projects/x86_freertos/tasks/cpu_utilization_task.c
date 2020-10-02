#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

static void print_task_info(void);

void cpu_utilization_print_task(void *p) {
  while (1) {
    print_task_info();
    vTaskDelay(1000);
  }
}

static void print_task_info(void) {
#if (0 != configUSE_TRACE_FACILITY)
  // Enum to char : eRunning, eReady, eBlocked, eSuspended, eDeleted
  static const char *const task_status_table[] = {"running", " ready ", "blocked", "suspend", "deleted"};

  // Limit the tasks to avoid heap allocation.
  const unsigned portBASE_TYPE max_tasks = 10;
  TaskStatus_t status[max_tasks];
  uint32_t total_cpu_runtime = 0;
  uint32_t total_tasks_runtime = 0;

  const uint32_t total_run_time = portGET_RUN_TIME_COUNTER_VALUE();
  const unsigned portBASE_TYPE task_count = uxTaskGetSystemState(&status[0], max_tasks, &total_cpu_runtime);

  printf("--------------------------------------------------------------\n");
  printf("%10s  Status Pr Stack CPU%%          Time\n", "Name");

  for (unsigned priority_number = 0; priority_number < configMAX_PRIORITIES; priority_number++) {
    /* Print in sorted priority order */
    for (unsigned i = 0; i < task_count; i++) {
      const TaskStatus_t *task = &status[i];
      if (task->uxBasePriority == priority_number) {
        total_tasks_runtime += task->ulRunTimeCounter;

        const unsigned cpu_percent = (0 == total_cpu_runtime) ? 0 : (task->ulRunTimeCounter * 100) / total_cpu_runtime;
        const unsigned time_us = task->ulRunTimeCounter;
        const unsigned stack_in_bytes = (sizeof(void *) * task->usStackHighWaterMark);

        printf("%10s %s %2u %5u %4u %10u us\n", task->pcTaskName, task_status_table[task->eCurrentState],
               (unsigned)task->uxBasePriority, stack_in_bytes, cpu_percent, time_us);
      }
    }
  }
#else
  printf("Unable to provide you the task information along with their CPU and stack usage.\n");
  printf("configUSE_TRACE_FACILITY macro at FreeRTOSConfig.h must be non-zero\n");
#endif
}