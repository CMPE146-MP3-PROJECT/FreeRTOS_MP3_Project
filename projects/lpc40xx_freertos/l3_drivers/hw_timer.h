#pragma once

#include <stdint.h>

#include "function_types.h"

/**
 * The type of timers supported by LPC40xx
 * @note For this SJ-2 project, lpc_timer1 is being used by to keep track of 'up time' (by sys_time.h)
 */
typedef enum {
  LPC_TIMER__0 = 0,
  LPC_TIMER__1,
  LPC_TIMER__2,
  LPC_TIMER__3,
} lpc_timer_e;

/// There are 4 match registers for HW timer on the LPC40xx
typedef enum {
  LPC_TIMER__MR0 = 0,
  LPC_TIMER__MR1,
  LPC_TIMER__MR2,
  LPC_TIMER__MR3,
} lpc_timer__mr_e;

/**
 * Enables and starts the timer
 * @param prescalar_divider This divider is applied to the clock source into the timer
 *                          This is offset by 1, so 0 means divide by 1, and 1 means divide by 2
 *
 * @param isr_callback      The ISR callback for the timer, including all Match-Register interrupts
 * @note The isr_callback may be NULL if the timer will not be configured for any match interrupts
 */
void hw_timer__enable(lpc_timer_e timer, const uint32_t prescalar_divider, function__void_f isr_callback);

/**
 * When the HW timer counts up and matches the mr_value of type lpc_timer__mr_e then it will:
 *   - Generate an interrupt and invoke the callback registered during hw_timer__enable()
 *
 * @note All match registers use the same interrupt registered through hw_timer__enable()
 */
void hw_timer__enable_match_isr(lpc_timer_e timer, lpc_timer__mr_e mr_type, const uint32_t mr_value);

/// When isr_callback fires that was set by hw_timer__enable(), then this should be used to clear the MR interrupt
void hw_timer__acknowledge_interrupt(lpc_timer_e timer, lpc_timer__mr_e mr_type);

/// @returns the HW timer value
uint32_t hw_timer__get_value(lpc_timer_e timer);

/// Sets (overwrites) the current HW timer value (TC register)
void hw_timer__set_value(lpc_timer_e timer, uint32_t tc_value);
