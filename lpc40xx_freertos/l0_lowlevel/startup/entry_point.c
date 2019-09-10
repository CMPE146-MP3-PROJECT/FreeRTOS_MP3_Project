#include "startup.h"

extern void main(void);

void cpu_startup_entry_point(void) {
  startup__initialize_ram();
  startup__initialize_fpu();
  startup__initialize_system_clock_96mhz();
  main();

  /**
   * main() should never return.
   * CPU will now halt forever at this point.
   */
  while (1) {
  }
}
