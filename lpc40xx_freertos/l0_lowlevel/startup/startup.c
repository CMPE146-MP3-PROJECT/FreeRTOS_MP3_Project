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
void startup__initialize_cpu(void) {
  SCB->CPACR |= (0xF << 20); // Set [23:20]

  // NVIC_SetPriorityGrouping(0);

  const int first_peripheral = (int)WDT_IRQn;
  const int last_peripheral = (int)EEPROM_IRQn;
  for (int peripheral = first_peripheral; peripheral <= last_peripheral; peripheral++) {
    // NVIC_SetPriority(peripheral, 31); // TODO: need to fix this
  }
}
