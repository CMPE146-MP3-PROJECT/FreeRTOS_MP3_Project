#include "clock.h"

#include "lpc40xx.h"

/**
 * FreeRTOS 'configCPU_CLOCK_HZ' references this code to get the CPU frequency.
 * Then, the Cortex-M4 port uses that reference to configure the RTOS tick rate.
 *
 * We use fixed CPU frequency because clock__initialize_system_clock_96mhz() configures
 * fixed CPU frequency. It is rare that anyone needs to change this (ever).
 */
uint32_t clock__get_core_clock_hz(void) { return (UINT32_C(96) * 1000 * 1000); }

// We set PCLKSEL to 1, and hence peripheral clock is the same as core clock
uint32_t clock__get_peripheral_clock_hz(void) { return clock__get_core_clock_hz(); }

void clock__initialize_system_clock_96mhz(void) {
  // Step 3 from UM: Write PLL new setup values to the PLL CFG register
  const uint8_t multiplier = 8; // 8 * 12Mhz = 96Mhz

  LPC_SC->PLL0CFG = ((multiplier - 1) << 0);
  LPC_SC->PLL0CON = 1; // PLL enable

  // Necessary feed sequence to ensure the changes are intentional
  LPC_SC->PLL0FEED = 0xAA;
  LPC_SC->PLL0FEED = 0x55;

  // Step 4 from UM: Setup necessary clock dividers
  LPC_SC->PCLKSEL = 1; // APB peripherals receive clock/1 (divided by 1)

  // Step 5 from UM: Wait for PLL to lock
  const uint32_t pll_lock_bit_mask = (1 << 10);
  while (!(LPC_SC->PLL0STAT & pll_lock_bit_mask)) {
    ;
  }

  // Before we switch to the faster clock, we need to configure flash memory access properly with respect to core clock
  uint32_t flash_cfg = LPC_SC->FLASHCFG;
  flash_cfg &= ~(0xF << 12);
  flash_cfg |= (0x4 << 12); // 5 cpu clocks for use with up to 100Mhz
  LPC_SC->FLASHCFG = flash_cfg;

  // Step 6 from UM: Connect the PLL to the CPU
  const uint32_t cpu_on_pll_clock = (1 << 8) | (1 << 0);
  LPC_SC->CCLKSEL = cpu_on_pll_clock; // CPU uses PLL clock (Table 20)
}
