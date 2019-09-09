#include "hw_timer.h"

void hw_timer__enable(lpc_timer_e timer, const uint32_t prescalar_divider, function_type__void isr_callback) {}

void hw_timer__enable_match_isr(lpc_timer_e timer, lpc_timer__mr_e mr_type, const uint32_t mr_value) {}

uint32_t hw_timer__get_value(lpc_timer_e timer) { return 0; }
