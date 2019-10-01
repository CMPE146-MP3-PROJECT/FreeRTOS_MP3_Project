#include <stdio.h>

#include "board_io.h"
#include "ssp2.h"

#include "clock.h"
#include "startup.h"
#include "sys_time.h"

#include "ff.h"

extern void main(void);
static void entry_point__halt(void);
static void entry_point__mount_sd_card(void);

void entry_point(void) {
  startup__initialize();

  clock__initialize_system_clock_96mhz();
  sys_time__init(clock__get_peripheral_clock_hz());
  board_io__initialize();

  const uint32_t spi_sd_max_speed_khz = 24 * 1000;
  ssp2__initialize(spi_sd_max_speed_khz);
  entry_point__mount_sd_card();

  // Do not do any bufferring for standard input otherwise getchar(), scanf() may not work
  setvbuf(stdin, 0, _IONBF, 0);

  main();
  entry_point__halt();
}

static void entry_point__halt(void) {
  /**
   * main() should never return.
   * CPU will now halt forever at this point.
   */
  fprintf(stderr, "ERROR: main() should never return, program has been halted");
  while (1) {
    ;
  }
}

static void entry_point__mount_sd_card(void) {
  // This FATFS object should never go out of scope
  static FATFS sd_card_drive;

  const BYTE option_mount_later = 0; // Actually mounts later when the first file is accessed
  const TCHAR *default_drive = (const TCHAR *)"";

  f_mount(&sd_card_drive, default_drive, option_mount_later);
}
