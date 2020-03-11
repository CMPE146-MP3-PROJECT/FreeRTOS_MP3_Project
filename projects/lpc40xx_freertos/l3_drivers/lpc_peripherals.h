#pragma once

#include <stdbool.h>

#include "function_types.h"

/**
 * @file
 * This file provides API to turn on peripheral power, and their interrupts
 * Turning off power and disabling interrupts is a YAGNI so do not be suprised of the omission
 */

/**
 * These enum values map to the interrupt vector table
 * Check UM10562.pdf, Chapter 5: NVIC
 *
 * @note If you add more peripherals here, you will have to modify lpc_peripherals.c too
 *
 * @warning these need to map to the same values as lpc40xx.h 'IRQn_Type'; we duplicate it
 *          here to avoid including lpc40xx.h and make unit-testing and mocking easier.
 */
typedef enum {
  LPC_PERIPHERAL__TIMER0 = 1,
  LPC_PERIPHERAL__TIMER1 = 2,
  LPC_PERIPHERAL__TIMER2 = 3,
  LPC_PERIPHERAL__TIMER3 = 4,

  LPC_PERIPHERAL__UART0 = 5,
  LPC_PERIPHERAL__UART1 = 6,
  LPC_PERIPHERAL__UART2 = 7,
  LPC_PERIPHERAL__UART3 = 8,

  LPC_PERIPHERAL__I2C0 = 10,
  LPC_PERIPHERAL__I2C1 = 11,
  LPC_PERIPHERAL__I2C2 = 12,

  /**
   * CAN0 and CAN1 do not have consistency like the other peripherals
   * CAN0 and CAN1 share the same interrupt, but have separate power controls
   * We arbritarily make up CAN1 as +64 offset but handle it at lpc_peripherals.c
   */
  LPC_PERIPHERAL__CAN0 = 25,
  LPC_PERIPHERAL__CAN1 = 25 + 64,

  LPC_PERIPHERAL__PWM1 = 9,
  LPC_PERIPHERAL__ADC = 22,
  LPC_PERIPHERAL__GPIO = (54 - 16),

  LPC_PERIPHERAL__SSP0 = 14,
  LPC_PERIPHERAL__SSP1 = 15,
  LPC_PERIPHERAL__SSP2 = 36,

  LPC_PERIPHERAL__GPDMA = 26,
} lpc_peripheral_e;

void lpc_peripheral__turn_on_power_to(lpc_peripheral_e peripheral);
bool lpc_peripheral__is_powered_on(lpc_peripheral_e peripheral);

void lpc_peripheral__enable_interrupt(lpc_peripheral_e peripheral, function__void_f isr_callback,
                                      const char *name_for_rtos_trace);
