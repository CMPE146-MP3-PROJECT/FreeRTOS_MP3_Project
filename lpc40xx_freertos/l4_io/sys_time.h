#pragma once

#include <stdint.h>

/**
 * This will initialize the system time module to track the 'uptime'
 * CPU clock must be at least 1Mhz as we setup a 1uS HW timer to keep track of uptime
 */
void sys_time__init(uint32_t peripheral_clock_hz);

/**
 * @returns a 64-bit uptime value in microseconds
 * We assume that this will never overflow in the lifetime of a program: 18,446,744,073,709,551,615 uS = 584,942 years
 */
uint64_t sys_time__get_uptime_us(void);

uint64_t sys_time__get_uptime_ms(void);
