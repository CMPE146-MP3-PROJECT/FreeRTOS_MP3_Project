#include "gpio_lab.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
//#include <cstdint.h>

/// Should alter the hardware registers to set the pin as input
void gpio__set_as_input(port_pin_s pin_num) {
  if (pin_num.port == 0) {
    LPC_GPIO0->DIR &= ~(1 << pin_num.pin);
  } else if (pin_num.port == 1) {
    LPC_GPIO1->DIR &= ~(1 << pin_num.pin);
  } else if (pin_num.port == 3) {
    LPC_GPIO2->DIR &= ~(1 << pin_num.pin);
  }
}

/// Should alter the hardware registers to set the pin as output
void gpio__set_as_output(port_pin_s pin_num) { LPC_GPIO1->DIR |= (1 << pin_num); }

/// Should alter the hardware registers to set the pin as high
void gpio__set_high(port_pin_s pin_num) { LPC_GPIO2->SET = (1 << pin_num); }

/// Should alter the hardware registers to set the pin as low
void gpio__set_low(port_pin_s pin_num) { LPC_GPIO2->CLR = (1 << pin_num); }

void gpio__set(port_pin_s pin_num, bool high) {
  if (high == 1) {
    gpio__set_high(pin_num);
  } else {
    gpio__set_low(pin_num);
  }
}
bool gpio__get_level(port_pin_s pin_num) {
  if (LPC_GPIO0->PIN & (1 << pin_num) == 1) {
    return 1;
  } else
    return 0;
}