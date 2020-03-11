#include <stdio.h>

#include "clock.h"
#include "crash.h"
#include "peripherals_init.h"
#include "sensors.h"
#include "startup.h"
#include "sys_time.h"
#include "trcRecorder.h"

extern void main(void);
static void entry_point__halt(void);
static void entry_point__handle_crash_report(void);

void entry_point(void) {
  startup__initialize_ram();
  startup__initialize_fpu();
  startup__initialize_interrupts();

  clock__initialize_system_clock_96mhz();
  sys_time__init(clock__get_peripheral_clock_hz());

  /* RTOS trace is an optional component enabled by the following macro at FreeRTOSConfig.h
   *  - configUSE_TRACE_FACILITY
   * We need to initialize the trace early before using ANY RTOS API
   */
  vTraceEnable(TRC_INIT);

  // Peripherals init initializes UART and then we can print the crash report if applicable
  peripherals_init();
  entry_point__handle_crash_report();

  if (!sensors__init()) {
    printf("\n%s(): WARNING: Sensor errors on this board\n", __FUNCTION__);
  }

  // If RTOS trace is compiled in (configUSE_TRACE_FACILITY) then enable it at this point before entering main()
  vTraceEnable(TRC_START);

  printf("\n%s(): Entering main()\n", __FUNCTION__);
  main();

  entry_point__halt();
}

static void entry_point__halt(void) {
  // main() should never return: CPU will now halt forever at this point.
  fprintf(stderr, "ERROR: main() should never return, program has been halted");
  while (1) {
  }
}

static void entry_point__handle_crash_report(void) {
  if (crash__report_if_occurred()) {
    while (sys_time__get_uptime_ms() < 5000) {
      ; // Deliberately delay the startup and let the user carefully read the information
    }
  }
}
