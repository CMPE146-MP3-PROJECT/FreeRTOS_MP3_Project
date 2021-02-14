#include "gpio_lab.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
//#include <cstdint.h>

/// Should alter the hardware registers to set the pin as input
void gpio0__set_as_input(port_pin_s pin_num) {
  switch (pin_num.port) {
  case 0:
    LPC_GPIO0->DIR &= ~(1 << pin_num.pin);
    break;
  case 1:
    LPC_GPIO1->DIR &= ~(1 << pin_num.pin);
    break;
  case 2:
    LPC_GPIO2->DIR &= ~(1 << pin_num.pin);
    break;
  }
}

/// Should alter the hardware registers to set the pin as output
void gpio0__set_as_output(uint8_t pin_num) { LPC_GPIO1->DIR |= (1 << pin_num); }

/// Should alter the hardware registers to set the pin as high
void gpio0__set_high(uint8_t pin_num) { LPC_GPIO2->SET = (1 << pin_num); }

/// Should alter the hardware registers to set the pin as low
void gpio0__set_low(uint8_t pin_num) { LPC_GPIO2->CLR = (1 << pin_num); }

void gpio0__set(uint8_t pin_num, bool high) {
  if (high == 1) {
    gpio0__set_high(pin_num);
  } else {
    gpio0__set_low(pin_num);
  }
}
bool gpio0__get_level(uint8_t pin_num) {
  if (LPC_GPIO0->PIN & (1 << pin_num) == 1)
    return 1;
}
else {return 0;}
}