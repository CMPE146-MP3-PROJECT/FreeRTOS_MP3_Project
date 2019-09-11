#include "sys_time.h"

#include "hw_timer.h"

/// sys_time module uses this HW timer
static const lpc_timer_e sys_time__hw_timer = lpc_timer0;
static const lpc_timer__mr_e sys_time__hw_timer_mr = lpc_timer__mr0;

static const uint32_t sys_time__us_per_sec = UINT32_C(1) * 1000 * 1000;
static uint32_t sys_time__sec_counter = 0;

static void sys_time__1sec_isr(void) {
  ++sys_time__sec_counter;
  hw_timer__acknowledge_interrupt(sys_time__hw_timer, sys_time__hw_timer_mr);
}

/*******************************************************************************
 *
 *                      P U B L I C    F U N C T I O N S
 *
 ******************************************************************************/

void sys_time__init(uint32_t cpu_clock_hz) {
  const uint32_t prescalar_for_1us = (cpu_clock_hz / sys_time__us_per_sec) - 1;

  // Enable the timer with 1uS resolution with an interrupt every one second
  hw_timer__enable(sys_time__hw_timer, prescalar_for_1us, sys_time__1sec_isr);
  hw_timer__enable_match_isr_and_reset(sys_time__hw_timer, sys_time__hw_timer_mr, sys_time__us_per_sec);
}

uint64_t sys_time__get_uptime_us(void) {
  uint32_t before_us = 0;
  uint32_t after_us = 0;
  uint32_t seconds = 0;

  /**
   * Loop until we can safely read both the rollover value and the timer value.
   * When the timer rolls over, the TC value will start from zero, and the 'after' value will be less than the 'before'
   * value in which case, we will loop again and pick up the new rollover count.  This avoid critical section and
   * simplifies the logic of reading sys_time__sec_counter and the 32-bit HW timer value.
   */
  do {
    before_us = hw_timer__get_value(sys_time__hw_timer);
    seconds = sys_time__sec_counter;
    after_us = hw_timer__get_value(sys_time__hw_timer);
  } while (after_us < before_us);

  uint64_t uptime_us = after_us;
  uptime_us += ((uint64_t)sys_time__us_per_sec * seconds);

  return uptime_us;
}
