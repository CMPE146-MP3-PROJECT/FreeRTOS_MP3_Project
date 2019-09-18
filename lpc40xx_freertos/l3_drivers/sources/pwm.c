#include "pwm.h"

#include "clock.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"

/*******************************************************************************
 *
 *                      P U B L I C    F U N C T I O N S
 *
 ******************************************************************************/

void pwm__init_single_edge(uint32_t frequency_in_hertz) {
  const uint32_t pwm_bits_9_to_14 = 0x3F;

  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__PWM1);
  pwm__set_frequency(frequency_in_hertz);

  LPC_PWM1->MCR |= (1 << 1); // Enable PWM reset when it matches MR0

  LPC_PWM1->TCR = (1 << 0) | (1 << 3); // Enable PWM counter

  LPC_PWM1->PCR |= (pwm_bits_9_to_14 << 9); // Enable the PWM (bits 9-14)

  return;
}

void pwm__set_frequency(uint32_t frequency_in_hertz) {
  if (0 == frequency_in_hertz) {
    frequency_in_hertz = 50; // Default to 50Hz
  }
  const uint32_t match_reg_value = (clock__get_core_clock_hz() / frequency_in_hertz);
  LPC_PWM1->MR0 = match_reg_value;

  return;
}

void pwm__set_duty_cycle(pwm_e pwm_pin, double duty_cycle_in_percent) {
  const uint32_t mr0_reg_val = LPC_PWM1->MR0;
  uint32_t match_reg_value = (mr0_reg_val * duty_cycle_in_percent) / 100;

  switch (pwm_pin) {
  case PWM__2_0:
    LPC_PWM1->MR1 = match_reg_value;
    break;
  case PWM__2_1:
    LPC_PWM1->MR2 = match_reg_value;
    break;
  case PWM__2_2:
    LPC_PWM1->MR3 = match_reg_value;
    break;
  case PWM__2_4:
    LPC_PWM1->MR5 = match_reg_value;
    break;
  case PWM__2_5:
    LPC_PWM1->MR6 = match_reg_value;
    break;
  default:
    break;
  }

  LPC_PWM1->LER |= (1 << (pwm_pin + 1)); // Enable Latch Register

  return;
}