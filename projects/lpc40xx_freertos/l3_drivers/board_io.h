#pragma once

#include <stdbool.h>

#include "gpio.h"

void board_io__initialize(void);

void board_io__sd_card_cs(void);
void board_io__sd_card_ds(void);
bool board_io__sd_card_is_present(void);

gpio_s board_io__get_led0(void);
gpio_s board_io__get_led1(void);
gpio_s board_io__get_led2(void);
gpio_s board_io__get_led3(void);
