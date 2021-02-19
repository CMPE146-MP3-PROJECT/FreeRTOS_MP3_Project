// @file gpio_isr.c
#include "gpio_isr.h"

// Note: You may want another separate array for falling vs. rising edge callbacks
static function_pointer_t gpiox_callbacks_falling[32];
static function_pointer_t gpiox_callbacks_rising[32];

void gpiox__attach_interrupt(port_pin_s pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  // 1) Store the callback based on the pin at gpio0_callbacks
  // 2) Configure GPIO 0 pin for rising or falling edge
  if (interrupt_type == GPIO_INTR__FALLING_EDGE) {
    gpiox__trigger_level(pin, 0);
    callback = gpiox_callbacks_falling[pin.pin];
  } else if (interrupt_type == GPIO_INTR__RISING_EDGE) {
    gpiox__trigger_level(pin, 1);
    callback = gpiox_callbacks_rising[pin.pin];
  }
}

// We wrote some of the implementation for you
void gpiox__interrupt_dispatcher(void) {
  // Check which pin generated the interrupt
  port_pin_trigger temp_status;
  function_pointer_t attached_user_handler;

  temp_status = check_interrupt_pin();
  const int pin_that_generated_interrupt = temp_status.port_pin.pin;
  if (temp_status.trigger) {
    /// Trigger from Rising Edge
    attached_user_handler = gpiox_callbacks_rising[pin_that_generated_interrupt];
  } else {
    attached_user_handler = gpiox_callbacks_falling[pin_that_generated_interrupt];
  }
  // Invoke the user registered callback, and then clear the interrupt
  attached_user_handler();
  clear_pin_interrupt(pin_that_generated_interrupt);
}

port_pin_trigger check_interrupt_pin() {
  port_pin_s temp_gpio = {0, 0};
  port_pin_trigger save_pin_trigger_level = {temp_gpio, 0};
  // check for port0 falling.............................................
  if (Check_all_Int_Status()) {

    for (int i = 0; i <= 31; i++) {
      // fprintf(stderr, "Recent int port pin: p%u_%u\n", temp_gpio.port, temp_gpio.pin);

      if (Check_signle_Int_Status(temp_gpio, 0) == 1) {
        // fprintf(stderr, "Port0: Found interrupt from Falling Edge!\n");
        save_pin_trigger_level.trigger = 0;
        save_pin_trigger_level.port_pin = temp_gpio;

        return save_pin_trigger_level;
      }
      temp_gpio.pin++;
    }

    // check for port0 rising
    temp_gpio.pin = 0; // reset pin number
    for (int i = 0; i <= 31; i++) {
      // fprintf(stderr, "Recent int port pin: p%u_%u\n", temp_gpio.port, temp_gpio.pin);

      if (Check_signle_Int_Status(temp_gpio, 1) == 1) {

        // fprintf(stderr, "Port0: Found interrupt from Rising Edge!\n");
        save_pin_trigger_level.trigger = 1;
        save_pin_trigger_level.port_pin = temp_gpio;

        return save_pin_trigger_level;
      }
      temp_gpio.pin++;
    }
    // check for port2.....................................................
    temp_gpio.port = 2; // reset and prepare for port2
    temp_gpio.pin = 0;
    // check for port2 falling
    for (int i = 0; i <= 31; i++) {
      // fprintf(stderr, "Recent int port pin: p%u_%u\n", temp_gpio.port, temp_gpio.pin);

      if (Check_signle_Int_Status(temp_gpio, 0) == 1) {

        // fprintf(stderr, "Port2: Found interrupt from Falling Edge!\n");
        save_pin_trigger_level.trigger = 0;
        save_pin_trigger_level.port_pin = temp_gpio;

        return save_pin_trigger_level;
      }
      temp_gpio.pin++;
    }

    // check for port2 rising
    temp_gpio.pin = 0; // reset pin number
    for (int i = 0; i <= 31; i++) {
      // fprintf(stderr, "Recent int port pin: p%u_%u\n", temp_gpio.port, temp_gpio.pin);

      if (Check_signle_Int_Status(temp_gpio, 1) == 1) {

        // fprintf(stderr, "Port2: Found interrupt from Rising Edge!\n");
        save_pin_trigger_level.trigger = 1;
        save_pin_trigger_level.port_pin = temp_gpio;

        return save_pin_trigger_level;
      }
      temp_gpio.pin++;
    }

  } else {
    fprintf(stderr, "Interrput No Found!!\n");
  }

  return save_pin_trigger_level;
}