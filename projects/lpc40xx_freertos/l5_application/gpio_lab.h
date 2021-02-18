// file gpio_lab.h
#pragma once
#include "lpc40xx.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t port;
  uint8_t pin;
} port_pin_s;

/// Should alter the hardware registers to set the pin as input
void gpiox__set_as_input(port_pin_s pin_num);

/// Should alter the hardware registers to set the pin as output
void gpiox__set_as_output(port_pin_s pin_num);

/// Should alter the hardware registers to set the pin as high
void gpiox__set_high(port_pin_s pin_num);

/// Should alter the hardware registers to set the pin as low
void gpiox__set_low(port_pin_s pin_num);

void gpiox__set(port_pin_s pin_num, bool high);

bool gpiox__get_level(port_pin_s pin_num);

void gpiox__trigger_level(port_pin_s gpio_pin, int 1_or_0);