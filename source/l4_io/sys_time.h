#pragma once

/// This will initialize the system time module to track the 'uptime'
void sys_time__init(void);

/// @returns a 64-bit uptime value in microseconds
uint64_t sys_time__get_uptime_us(void);
