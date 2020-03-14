#include "board_io.h"

#include "gpio.h"
#include "lpc40xx.h"

/**
 * Board information:
 *
 * P2.3  - LED0
 * P1.26 - LED1
 * P1.24 - LED2
 * P1.18 - LED3
 *
 * P1.19 - SW0
 * P1.15 - SW1
 * P0.30 - SW2
 * P0.29 - SW3
 *
 * P1.8 - SD CS
 * P1.9 - SD card detect
 * P1.10 - external flash CS
 *
 * P1.0 - SSP2 - SCK
 * P1.1 - SSP2 - MOSI
 * P1.4 - SSP2 - MISO
 *
 * P0.10 - SDA2
 * P0.11 - SCL2
 */

static const uint32_t board_io__sd_card_cs_pin = (1 << 8);
static const uint32_t board_io__sd_card_detect_pin = (1 << 9);
static gpio_s board_io__led0, board_io__led1, board_io__led2, board_io__led3;
static gpio_s board_io__sw0, board_io__sw1, board_io__sw2, board_io__sw3;

void board_io__initialize(void) {
  // Note: return type of gpio__construct_with_function() because we do not need GPIO instance after its configuration

  gpio__construct_with_function(GPIO__PORT_0, 2, GPIO__FUNCTION_1); // P0.2 - Uart-0 Tx
  gpio__construct_with_function(GPIO__PORT_0, 3, GPIO__FUNCTION_1); // P0.3 - Uart-0 Rx

  // SPI bus 2 (SSP2)
  gpio__construct_with_function(GPIO__PORT_1, 0, GPIO__FUNCTION_4); // P1.0 - SCK2
  gpio__construct_with_function(GPIO__PORT_1, 1, GPIO__FUNCTION_4); // P1.1 - MOSI2
  gpio__construct_with_function(GPIO__PORT_1, 4, GPIO__FUNCTION_4); // P1.4 - MISO2

  // SD card
  gpio__construct_as_output(GPIO__PORT_1, 8); // SD card CS
  board_io__sd_card_ds();
  gpio__construct_as_input(GPIO__PORT_1, 9); // SD card detect

  // I2C pins; these require open-drain mode to be enabled
  const gpio_s sda_2 = gpio__construct_with_function(GPIO__PORT_0, 10, GPIO__FUNCTION_2); // P0.10 - SDA
  const gpio_s scl_2 = gpio__construct_with_function(GPIO__PORT_0, 11, GPIO__FUNCTION_2); // P0.11 - SCL
  gpio__enable_open_drain(sda_2);
  gpio__enable_open_drain(scl_2);

  // Output pins
  board_io__led0 = gpio__construct_as_output(GPIO__PORT_2, 3);
  board_io__led1 = gpio__construct_as_output(GPIO__PORT_1, 26);
  board_io__led2 = gpio__construct_as_output(GPIO__PORT_1, 24);
  board_io__led3 = gpio__construct_as_output(GPIO__PORT_1, 18);

  // Input pins
  board_io__sw0 = gpio__construct_as_input(GPIO__PORT_1, 19);
  board_io__sw1 = gpio__construct_as_input(GPIO__PORT_1, 15);
  board_io__sw2 = gpio__construct_as_input(GPIO__PORT_0, 30);
  board_io__sw3 = gpio__construct_as_input(GPIO__PORT_0, 29);

  // SW0 and SW1 require internal pull down resistors
  // otherwise undefined behavior will result from
  // floating pins on open switch
  gpio__enable_pull_down_resistors(board_io__sw0);
  gpio__enable_pull_down_resistors(board_io__sw1);
}

// Note: Not using gpio.h API here to optimize the speed of SSP CS selection
void board_io__sd_card_cs(void) { LPC_GPIO1->CLR = board_io__sd_card_cs_pin; }
void board_io__sd_card_ds(void) { LPC_GPIO1->SET = board_io__sd_card_cs_pin; }

bool board_io__sd_card_is_present(void) {
  const uint32_t card_present_bitmask = (LPC_GPIO1->PIN & board_io__sd_card_detect_pin);
  return (0 == card_present_bitmask); // Signal is active low
}

gpio_s board_io__get_led0(void) { return board_io__led0; }
gpio_s board_io__get_led1(void) { return board_io__led1; }
gpio_s board_io__get_led2(void) { return board_io__led2; }
gpio_s board_io__get_led3(void) { return board_io__led3; }

gpio_s board_io__get_sw0(void) { return board_io__sw0; }
gpio_s board_io__get_sw1(void) { return board_io__sw1; }
gpio_s board_io__get_sw2(void) { return board_io__sw2; }
gpio_s board_io__get_sw3(void) { return board_io__sw3; }
