#include "pwm1.h"

#include "clock.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"

void pwm1__init_single_edge(uint32_t frequency_in_hertz) {
  const uint32_t pwm_channel_output_enable_mask = 0x3F;
  const uint32_t default_frequency_in_hertz = 50;
  uint32_t valid_frequency_in_hertz = frequency_in_hertz;

  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__PWM1);

  if (0 == frequency_in_hertz) {
    valid_frequency_in_hertz = default_frequency_in_hertz;
  }
  const uint32_t match_reg_value = (clock__get_peripheral_clock_hz() / valid_frequency_in_hertz);

  // MR0 holds the value that Timer Counter should count upto
  // This will get us the desired PWM pulses per second
  // Ex: If CPU freq = 10Hz, desired frequency = 2Hz
  // MR0 = 10/2 = 5. This means, TC will count upto 5 per pulse, generating 2 pulse per second
  if (match_reg_value > 0) {
    LPC_PWM1->MR0 = (match_reg_value - 1);
  }

  LPC_PWM1->MCR |= (1 << 1);                              ///< Enable PWM reset when it matches MR0
  LPC_PWM1->TCR = (1 << 0) | (1 << 3);                    ///< Enable PWM counter
  LPC_PWM1->PCR |= (pwm_channel_output_enable_mask << 9); ///< Enable the PWM (bits 9-14)
}

void pwm1__set_duty_cycle(pwm1_channel_e pwm1_channel, float duty_cycle_in_percent) {
  const uint32_t mr0_reg_val = LPC_PWM1->MR0;
  const uint32_t match_reg_value = (mr0_reg_val * duty_cycle_in_percent) / 100;

  switch (pwm1_channel) {
  case PWM1__2_0:
    LPC_PWM1->MR1 = match_reg_value;
    break;
  case PWM1__2_1:
    LPC_PWM1->MR2 = match_reg_value;
    break;
  case PWM1__2_2:
    LPC_PWM1->MR3 = match_reg_value;
    break;
  case PWM1__2_4:
    LPC_PWM1->MR5 = match_reg_value;
    break;
  case PWM1__2_5:
    LPC_PWM1->MR6 = match_reg_value;
    break;
  default:
    break;
  }

  LPC_PWM1->LER |= (1 << (pwm1_channel + 1)); ///< Enable Latch Register
}
