#include "gpio_lab.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
//#include <cstdint.h>

/// Should alter the hardware registers to set the pin as input
void gpiox__set_as_input(port_pin_s pin_num) {
  if (pin_num.port == 0) {
    LPC_GPIO0->DIR &= ~(1 << pin_num.pin);
  } else if (pin_num.port == 1) {
    LPC_GPIO1->DIR &= ~(1 << pin_num.pin);
  } else if (pin_num.port == 2) {
    LPC_GPIO2->DIR &= ~(1 << pin_num.pin);
  }
}

/// Should alter the hardware registers to set the pin as output
void gpiox__set_as_output(port_pin_s pin_num) {
  if (pin_num.port == 0) {
    LPC_GPIO0->DIR |= (1 << pin_num.pin);
  } else if (pin_num.port == 1) {
    LPC_GPIO1->DIR |= (1 << pin_num.pin);
  } else if (pin_num.port == 2) {
    LPC_GPIO2->DIR |= (1 << pin_num.pin);
  }
}

/// Should alter the hardware registers to set the pin as high
void gpiox__set_high(port_pin_s pin_num) {
  if (pin_num.port == 0) {
    LPC_GPIO0->SET = (1 << pin_num.pin);
  } else if (pin_num.port == 1) {
    LPC_GPIO1->SET = (1 << pin_num.pin);
  } else if (pin_num.port == 2) {
    LPC_GPIO2->SET = (1 << pin_num.pin);
  }
}

/// Should alter the hardware registers to set the pin as low
void gpiox__set_low(port_pin_s pin_num) {
  if (pin_num.port == 0) {
    LPC_GPIO0->CLR = (1 << pin_num.pin);
  } else if (pin_num.port == 1) {
    LPC_GPIO1->CLR = (1 << pin_num.pin);
  } else if (pin_num.port == 2) {
    LPC_GPIO2->CLR = (1 << pin_num.pin);
  }
}

void gpiox__set(port_pin_s pin_num, bool high) {
  if (high == 1) {
    gpiox__set_high(pin_num);
  } else {
    gpiox__set_low(pin_num);
  }
}
bool gpiox__get_level(port_pin_s pin_num) {
  switch (pin_num.port) {
  case 0:
    return (LPC_GPIO0->PIN & (1 << pin_num.pin));
  case 1:
    return (LPC_GPIO1->PIN & (1 << pin_num.pin));
  case 2:
    return (LPC_GPIO2->PIN & (1 << pin_num.pin));
  }
  return false;
}