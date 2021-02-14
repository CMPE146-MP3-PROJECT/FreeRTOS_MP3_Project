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
void gpio0__set_as_input(port_pin_s pin_num);

/// Should alter the hardware registers to set the pin as output
void gpio0__set_as_output(uint8_t pin_num);

/// Should alter the hardware registers to set the pin as high
void gpio0__set_high(uint8_t pin_num);

/// Should alter the hardware registers to set the pin as low
void gpio0__set_low(uint8_t pin_num);

void gpio0__set(uint8_t pin_num, bool high);

bool gpio0__get_level(uint8_t pin_num);