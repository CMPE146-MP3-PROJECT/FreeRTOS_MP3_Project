// @file gpio_isr.c
#include "gpio_isr.h"
#include <stdio.h>
#include <stdlib.h>

// Note: You may want another separate array for falling vs. rising edge callbacks
static function_pointer_t gpio0_callbacks[32];
static function_pointer_t gpio2_callbacks[32];

// static function_pointer_t gpio0_callbacks_rising[32];
// static function_pointer_t gpio2_callbacks_falling[32];
// static function_pointer_t gpio2_callbacks_rising[32];

void gpiox__attach_interrupt(port_pin_s pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  // 1) Store the callback based on the pin at gpio0_callbacks
  // 2) Configure GPIO 0 pin for rising or falling edge
  if (interrupt_type == GPIO_INTR__FALLING_EDGE) {
    if (pin.port == 0) {
      gpio0_callbacks[pin.pin] = callback;
      gpiox__trigger_level(pin, 0); // enable 对应 pin 的 falling edge interrupt
    } else if (pin.port == 2) {
      gpio2_callbacks[pin.pin] = callback;
      gpiox__trigger_level(pin, 0);
    }
  } else if (interrupt_type == GPIO_INTR__RISING_EDGE) {
    if (pin.port == 0) {
      gpio0_callbacks[pin.pin] = callback;
      gpiox__trigger_level(pin, 1); // enable 对应 pin 的 rising edge interrupt
    } else if (pin.port == 2) {
      gpio2_callbacks[pin.pin] = callback;
      gpiox__trigger_level(pin, 1);
    }
  }
}
// We wrote some of the implementation for you
void gpiox__interrupt_dispatcher(void) {
  // Check which pin generated the interrupt
  port_pin_s result_pin;
  result_pin = check_all_pin_INT_status();
  function_pointer_t attached_user_handler = gpio0_callbacks[result_pin.pin];
  // Invoke the user registered callback, and then clear the interrupt
  attached_user_handler(); // ISR
  clear_pin_interrupt(result_pin);
}

port_pin_s check_all_pin_INT_status() {
  port_pin_s temp_port0_pinx = {0, 0};
  // port_pin_toge INT_pin = {temp_port0_pinx};
  // if (LPC_GPIOINT->IntStatus & 0xFFFFFFFF == 1) {
  for (int i = 0; i <= 31; i++) {
    if (get_pin_INT_status(temp_port0_pinx, 0) == 1) {
      // INT_pin.port_pin = temp_port0_pinx;
      return temp_port0_pinx;
    } else if (get_pin_INT_status(temp_port0_pinx, 1) == 1) {
      // INT_pin.port_pin = temp_port0_pinx;
      return temp_port0_pinx;
    }
    temp_port0_pinx.pin++;
  }
  //} else {
  // fprintf(stderr, "no interrupt available\n");
  //}
}
void clear_pin_interrupt(port_pin_s pin) {
  if (pin.port == 0) {
    LPC_GPIOINT->IO0IntClr = (1 << pin.pin);
  } else if (pin.port == 2) {
    LPC_GPIOINT->IO2IntClr = (1 << pin.pin);
  }
}
