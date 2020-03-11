
#include <stdio.h>

#include "board_io.h"
#include "common_macros.h"
#include "delay.h"
#include "ff.h"
#include "i2c.h"
#include "ssp2.h"
#include "uart.h"

static void peripherals_init__startup_sequence(void);
static const char *peripherals_init__mount_sd_card(void);
static void peripherals_init__uart0_init(void);
static void peripherals_init__i2c_init(void);

void peripherals_init(void) {
  board_io__initialize();
  peripherals_init__startup_sequence();

  // UART initialization is required in order to use <stdio.h> puts, printf() etc; @see system_calls.c
  peripherals_init__uart0_init();

  const uint32_t spi_sd_max_speed_khz = 24 * 1000;
  ssp2__initialize(spi_sd_max_speed_khz);
  const char *mount_info = peripherals_init__mount_sd_card();

  // UART is initialized, so we can now start using printf()
  const char *line = "--------------------------------------------------------------------------------";
  printf("\n%s\n%s(): Low level startup\n%s\n", line, __FUNCTION__, mount_info);

  peripherals_init__i2c_init();
}

static void peripherals_init__startup_sequence(void) {
  const gpio_s leds[] = {board_io__get_led0(), board_io__get_led1(), board_io__get_led2(), board_io__get_led3()};
  for (size_t number = 0; number < 32; number++) {
    gpio__toggle(leds[number % ARRAY_SIZE(leds)]);
    delay__ms(50);
  }
}

static const char *peripherals_init__mount_sd_card(void) {
  // This FATFS object should never go out of scope
  static FATFS sd_card_drive;
  const char *mount_info = "";

  const BYTE option_mount_now = 1;
  const TCHAR *default_drive = (const TCHAR *)"";

  if (FR_OK == f_mount(&sd_card_drive, default_drive, option_mount_now)) {
    mount_info = ("SD card mounted successfully\n");
  } else {
    mount_info = ("WARNING: SD card could not be mounted\n");
  }

  return mount_info;
}

static void peripherals_init__uart0_init(void) {
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

static void peripherals_init__i2c_init(void) {
  const uint32_t i2c_speed_hz = UINT32_C(400) * 1000;
  i2c__initialize(I2C__2, i2c_speed_hz, clock__get_peripheral_clock_hz());

  for (unsigned slave_address = 2; slave_address <= 254; slave_address += 2) {
    if (i2c__detect(I2C__2, slave_address)) {
      printf("I2C slave detected at address: 0x%02X\n", slave_address);
    }
  }
}
