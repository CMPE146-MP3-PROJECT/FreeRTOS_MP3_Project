#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * Creates all of the periodic tasks and the periodic task monitor task
 * After the RTOS starts, it will start to invoke the functions defined at periodic_callbacks.h
 *
 * @param task_stack_size
 * All periodic tasks, and the periodic task monitor are created with this amount of stack space
 */
void periodic_scheduler__initialize(uint32_t task_stack_size, bool run_1000hz);