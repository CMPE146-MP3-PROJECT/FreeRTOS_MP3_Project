#pragma once

#include <stdint.h>

typedef void (*periodic_callbacks_f)(uint32_t);

/**
 * @{
 * @name These callbacks are invoked by periodic_scheduler.c
 *
 * @param callback_count
 * This is an incrementing counter, so when it is 3 for 1Hz, then 3 seconds have elapsed
 * It is specific to each callback, so 3 for 10Hz, means 300ms have elapsed
 */
void periodic_callbacks__initialize(void); ///< Invoked once by periodic_scheduler__initialize()

void periodic_callbacks__1Hz(uint32_t callback_count);
void periodic_callbacks__10Hz(uint32_t callback_count);
void periodic_callbacks__100Hz(uint32_t callback_count);
void periodic_callbacks__1000Hz(uint32_t callback_count);
/** @} */
