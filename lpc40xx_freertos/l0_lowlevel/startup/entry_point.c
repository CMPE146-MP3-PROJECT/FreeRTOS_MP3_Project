#include "startup.h"

// note: This layer violates directory structure due to the low level initialization that needs to take place
#include "clock.h"    // Layer violation
#include "sys_time.h" // Layer violation

extern void main(void);

void cpu_startup_entry_point(void) {
  startup__initialize_ram();
  startup__initialize_fpu();

  clock__initialize_system_clock_96mhz();
  sys_time__init(clock__get_core_clock_hz());

  main();

  /**
   * main() should never return.
   * CPU will now halt forever at this point.
   */
  while (1) {
  }
}
