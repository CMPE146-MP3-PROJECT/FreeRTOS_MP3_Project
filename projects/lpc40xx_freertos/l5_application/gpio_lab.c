#include "gpio_lab.h"
#include "lpc40xx.h"
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
  if (pin_num.port == 0) {
    return (LPC_GPIO0->PIN & (1 << pin_num.pin));
  } else if (pin_num.port == 1) {
    return (LPC_GPIO1->PIN & (1 << pin_num.pin));
  } else if (pin_num.port == 2) {
    return (LPC_GPIO2->PIN & (1 << pin_num.pin));
  }
  return false;
}

void gpiox__trigger_level(port_pin_s gpio_pin, int posedge_or_negedge) {
  // only port 0 and port 2 are support interrupt
  if (gpio_pin.port == 0) {
    if (posedge_or_negedge == 0) {
      LPC_GPIOINT->IO0IntEnF |= (1 << gpio_pin.pin);
    } else {
      LPC_GPIOINT->IO0IntEnR |= (1 << gpio_pin.pin);
    }
  } else if (gpio_pin.port == 2) {
    if (posedge_or_negedge == 0) {
      LPC_GPIOINT->IO2IntEnF |= (1 << gpio_pin.pin);
    } else {
      LPC_GPIOINT->IO2IntEnR |= (1 << gpio_pin.pin);
    }
  }
}

bool get_pin_INT_status(port_pin_s pin_num, int rising_or_falling) {
  if (rising_or_falling == 0) {
    return (LPC_GPIOINT->IO0IntStatF & (1 << pin_num.pin));
  } else {
    return (LPC_GPIOINT->IO0IntStatR & (1 << pin_num.pin));
  }
}

/*bool get_pin_INT_status(port_pin_s pin_num, int rising_or_falling) {
  if (pin_num.port == 0) {
    if (rising_or_falling == 0) {
      return (LPC_GPIOINT->IO0IntStatF & (1 << pin_num.pin));
    } else {
      return (LPC_GPIOINT->IO0IntStatR & (1 << pin_num.pin));
    }
  } else if (pin_num.port == 2) {
    if (rising_or_falling == 0) {
      return (LPC_GPIOINT->IO2IntStatF & (1 << pin_num.pin));
    } else {
      return (LPC_GPIOINT->IO2IntStatR & (1 << pin_num.pin));
    }
  } else {
    fprintf(stderr, "port error\n");
  }
}*/