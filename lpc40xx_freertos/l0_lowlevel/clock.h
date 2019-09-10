#pragma once

#include <stdint.h>

uint32_t clock__get_core_clock_hz(void);

void clock__initialize_system_clock_96mhz(void);