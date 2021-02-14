#include "gpio_lab.h"
#include <inttypes.h>
//#include <cstdint.h>

/// Should alter the hardware registers to set the pin as input
void gpiox__set_as_input(port_pin_s pin_num) {
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
void gpiox__set_as_output(uint8_t pin_num) { LPC_GPIO1->DIR |= (1 << pin_num); }

/// Should alter the hardware registers to set the pin as high
void gpiox__set_high(uint8_t pin_num) { LPC_GPIO2->SET = (1 << pin_num); }

/// Should alter the hardware registers to set the pin as low
void gpiox__set_low(uint8_t pin_num) { LPC_GPIO2->CLR = (1 << pin_num); }

/*(void gpiox__set(port_pin_s pin_num, bool high) {
  if (high == 1) {
    gpiox__set_high(int pin_num);
  } else {
    gpiox__set_low(int pin_num);
  }
}*/
