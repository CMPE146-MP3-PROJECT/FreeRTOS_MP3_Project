#include "clock.h"

uint32_t clock__get_core_clock_hz(void)
{
    static const uint32_t core_clock = 12000000UL;
    return core_clock;
}
