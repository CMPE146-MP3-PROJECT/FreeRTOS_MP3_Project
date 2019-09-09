#pragma once

#include <stdint.h>

/**
 * This will initialize the system time module to track the 'uptime'
 * CPU clock must be at least 1Mhz as we setup a 1uS HW timer to keep track of uptime
 */
void sys_time__init(uint32_t cpu_clock_hz);

/// @returns a 64-bit uptime value in microseconds
uint64_t sys_time__get_uptime_us(void);
