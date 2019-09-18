/*
 * Pulse Width Modulation (PWM) driver
 */
#pragma once

#include <stdint.h>

// Only the below PWM1 pins are available on SJ2 board
// P2.0, P2.1, P2.2, P2.4, P2.5
typedef enum {
  PWM__2_0 = 0,
  PWM__2_1 = 1,
  PWM__2_2 = 2,
  PWM__2_4 = 4,
  PWM__2_5 = 5,
} pwm_e;

void pwm__init_single_edge(uint32_t frequency_in_hertz);
void pwm__set_frequency(uint32_t frequency_in_hertz);
void pwm__set_duty_cycle(pwm_e pwm_pin, double duty_cycle_in_percent);
