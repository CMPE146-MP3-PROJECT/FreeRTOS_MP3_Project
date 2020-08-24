#include <stdio.h>

#include "FreeRTOS.h"
#include "trcRecorder.h"

#include "clock.h"
#include "crash.h"
#include "peripherals_init.h"
#include "sensors.h"
#include "startup.h"
#include "sys_time.h"

extern void main(void);
static void entry_point__halt(void);
static void entry_point__handle_crash_report(void);
static void entry_point__rtos_trace_init_after_mounting_sd_card(void);

void entry_point(void) {
  startup__initialize_ram();
  startup__initialize_fpu();
  startup__initialize_interrupts();

  clock__initialize_system_clock_96mhz();
  sys_time__init(clock__get_peripheral_clock_hz());

  /* RTOS trace is an optional component configured by FreeRTOSConfig.h
   * We need to initialize the trace early before using ANY RTOS API
   *
   *  Notes:
   *  - We cannot use TRC_START here as the SD card is not initialized yet
   *  - This can be used even if the trace library is not enabled
   */
  vTraceEnable(TRC_INIT);

  // Peripherals init initializes UART and then we can print the crash report if applicable
  peripherals_init();
  entry_point__handle_crash_report();

  if (!sensors__init()) {
    printf("\n%s(): WARNING: Sensor errors on this board\n", __FUNCTION__);
  }

  entry_point__rtos_trace_init_after_mounting_sd_card();

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

static void entry_point__rtos_trace_init_after_mounting_sd_card(void) {
#if !defined(configENABLE_TRACE_ON_SD_CARD) || !defined(configUSE_TRACE_FACILITY)
#error "configENABLE_TRACE_ON_SD_CARD and configUSE_TRACE_FACILITY must be defined; try including FreeRTOSConfig.h"
// If instructed to trace on the SD card, then start the trace immediately
#elif (configENABLE_TRACE_ON_SD_CARD)
  vTraceEnable(TRC_START);
/* If we are not tracing to SD card, initialize the trace but do not start it.
 * However, since the vTraceEnable(TRC_INIT) occurs early, we are good here.
 */
#elif (configUSE_TRACE_FACILITY)
  // vTraceEnable(TRC_INIT);
#endif
}