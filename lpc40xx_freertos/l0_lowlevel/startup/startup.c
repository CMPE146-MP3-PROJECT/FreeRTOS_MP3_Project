#include "startup.h"

#include "lpc40xx.h"
#include "ram.h"

void startup__initialize_ram(void) {
  ram__init_data();
  ram__init_bss();
}

/**
 * References:
 * - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0439b/BEHBJHIG.html
 */
void startup__initialize_fpu(void) {
  SCB->CPACR |= (0xF << 20); // Set [23:20]
}

void startup__initialize_system_clock_96mhz(void) {
  // Step 3 from UM: Write PLL new setup values to the PLL CFG register
  const uint8_t multiplier = 8; // 8 * 12Mhz = 96Mhz

  LPC_SC->PLL0CFG = ((multiplier - 1) << 0);
  LPC_SC->PLL0CON = 1; // PLL enable

  // Necessary feed sequence to ensure the changes are intentional
  LPC_SC->PLL0FEED = 0xAA;
  LPC_SC->PLL0FEED = 0x55;

  // Step 4 from UM: Setup necessary clock dividers
  LPC_SC->PCLKSEL = 1; // APB peripherals receive clock/1 (divided by 1)

  const uint32_t pll_lock_bit_mask = (1 << 10);
  while (!(LPC_SC->PLL0STAT & pll_lock_bit_mask)) {
    ;
  }

  const uint32_t cpu_on_pll_clock = (1 << 8) | (1 << 0);
  LPC_SC->CCLKSEL = cpu_on_pll_clock; // CPU uses PLL clock (Table 20)
}
