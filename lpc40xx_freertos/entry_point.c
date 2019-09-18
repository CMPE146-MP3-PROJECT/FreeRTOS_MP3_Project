#include "board_io.h"
#include "clock.h"
#include "startup.h"
#include "sys_time.h"

extern void main(void);

void entry_point(void) {
  startup__initialize();

  clock__initialize_system_clock_96mhz();
  sys_time__init(clock__get_core_clock_hz());
  board_io__initialize();

  main();

  /**
   * main() should never return.
   * CPU will now halt forever at this point.
   */
  while (1) {
  }
}
