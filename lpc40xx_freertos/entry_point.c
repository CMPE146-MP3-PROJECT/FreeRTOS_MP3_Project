#include <stdio.h>

#include "ff.h"
#include "i2c.h"
#include "ssp2.h"
#include "uart.h"

#include "board_io.h"
#include "clock.h"
#include "startup.h"
#include "sys_time.h"

extern void main(void);
static void entry_point__halt(void);
static void entry_point__uart0_init(void);
static void entry_point__i2c_init(void);
static void entry_point__mount_sd_card(void);

void entry_point(void) {
  startup__initialize();

  clock__initialize_system_clock_96mhz();
  sys_time__init(clock__get_peripheral_clock_hz());
  board_io__initialize();

  const uint32_t spi_sd_max_speed_khz = 24 * 1000;
  ssp2__initialize(spi_sd_max_speed_khz);
  entry_point__mount_sd_card();

  /// UART initialization is required in order to use <stdio.h> puts, printf() etc; @see system_calls.c
  entry_point__uart0_init();

  // UART is initialized, so we can now start using printf()
  const char *line = "--------------------------------------------------------------------------------";
  printf("\n%s\n%s(): Low level startup\n", line, __FUNCTION__);

  entry_point__i2c_init();

  printf("\n%s(): Entering main()\n%s\n", __FUNCTION__, line);
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

static void entry_point__uart0_init(void) {
  // Do not do any bufferring for standard input otherwise getchar(), scanf() may not work
  setvbuf(stdin, 0, _IONBF, 0);

  // Note: PIN functions are initialized by board_io__initialize() for P0.2(Tx) and P0.3(Rx)
  uart__init(UART__0, clock__get_peripheral_clock_hz(), 115200);

  // You can use xQueueCreate() that uses malloc() as it is an easier API to work with, however, we opt to
  // use xQueueCreateStatic() to provide reference on how to create RTOS queue without dynamic memory allocation

  // Memory for the queue data structure
  static StaticQueue_t rxq_struct;
  static StaticQueue_t txq_struct;

  // Memory where the queue actually stores the data
  static uint8_t rxq_storage[32];
  static uint8_t txq_storage[128];

  // Make UART more efficient by backing it with RTOS queues (optional but highly recommended with RTOS)
  QueueHandle_t rxq_handle = xQueueCreateStatic(sizeof(rxq_storage), sizeof(char), rxq_storage, &rxq_struct);
  QueueHandle_t txq_handle = xQueueCreateStatic(sizeof(txq_storage), sizeof(char), txq_storage, &txq_struct);

  uart__enable_queues(UART__0, txq_handle, rxq_handle);
}

static void entry_point__i2c_init(void) {
  const uint32_t i2c_speed_hz = UINT32_C(400) * 1000;
  i2c__initialize(I2C__2, i2c_speed_hz, clock__get_peripheral_clock_hz());

  for (unsigned device = 2; device <= 254; device += 2) {
    if (i2c__detect(I2C__2, device)) {
      printf("I2C device detected at address: 0x%02X\n", device);
    }
  }
}

static void entry_point__mount_sd_card(void) {
  // This FATFS object should never go out of scope
  static FATFS sd_card_drive;

  const BYTE option_mount_later = 0; // Actually mounts later when the first file is accessed
  const TCHAR *default_drive = (const TCHAR *)"";

  f_mount(&sd_card_drive, default_drive, option_mount_later);
}
