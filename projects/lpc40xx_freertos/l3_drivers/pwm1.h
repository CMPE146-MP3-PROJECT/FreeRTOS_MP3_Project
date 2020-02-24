/*
 * Pulse Width Modulation (PWM) driver
 */
#pragma once

#include <stdint.h>

/// Only the below PWM1 channels are available on SJ2 board
typedef enum {
  PWM1__2_0 = 0, ///< Controls P2.0
  PWM1__2_1 = 1, ///< Controls P2.1
  PWM1__2_2 = 2, ///< Controls P2.2
  PWM1__2_4 = 4, ///< Controls P2.4
  PWM1__2_5 = 5, ///< Controls P2.5
} pwm1_channel_e;

/// For simple single edge PWM, all channels share the same frequency
void pwm1__init_single_edge(uint32_t frequency_in_hertz);
void pwm1__set_duty_cycle(pwm1_channel_e pwm1_channel, float duty_cycle_in_percent);
