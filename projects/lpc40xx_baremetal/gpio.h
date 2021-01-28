#pragma once

#include <stdbool.h>
#include <stdint.h>

/// Note that some ports may not be available on your microcontroller
typedef enum {
  GPIO__PORT_0 = 0,
  GPIO__PORT_1,
  GPIO__PORT_2,
  GPIO__PORT_3,
  GPIO__PORT_4,
  GPIO__PORT_5,
} gpio__port_e;

/**
 * Each pin on LPC40xx has up to 8 functions
 * The functions are specific to a pin and vary; only function 0 is common, which is a GPIO
 */
typedef enum {
  GPIO__FUNCITON_0_IO_PIN = 0,
  GPIO__FUNCTION_1,
  GPIO__FUNCTION_2,
  GPIO__FUNCTION_3,
  GPIO__FUNCTION_4,
  GPIO__FUNCTION_5,
  GPIO__FUNCTION_6,
  GPIO__FUNCTION_7,
} gpio__function_e;

/**
 * This struct is returned gpio__construct()
 * All further APIs need instance of this struct to operate
 */
typedef struct {
  gpio__port_e port_number : 3;
  uint8_t pin_number : 5;
} gpio_s;

/**
 * A pin must be constructed to use further API
 * @param pin_number_0_to_31 The pin number from 0-31
 */
gpio_s gpio__construct(gpio__port_e port, uint8_t pin_number_0_to_31);
gpio_s gpio__construct_as_input(gpio__port_e port, uint8_t pin_number_0_to_31);  ///< construct then set as input
gpio_s gpio__construct_as_output(gpio__port_e port, uint8_t pin_number_0_to_31); ///< construct then set as output

/// Construct and then set the pin function. In this case, DIR will not impact the pin
gpio_s gpio__construct_with_function(gpio__port_e port, uint8_t pin_number_0_to_31, gpio__function_e function);

/// Sets the function for a pin; @see gpio__function_e
void gpio__set_function(gpio_s gpio, gpio__function_e function);

/// Most of the pins are NOT open drain on power-up; this enables pin's open-collector mode
void gpio__enable_open_drain(gpio_s gpio);

/// Sets the function mode of a pin to enable pull down resistors
void gpio__enable_pull_down_resistors(gpio_s gpio);

/**
 * @{
 * @name Sets direction of the pin if used as an input/output pin
 * @warning This API is not thread safe
 */
void gpio__set_as_input(gpio_s gpio);
void gpio__set_as_output(gpio_s gpio);
/** @} */

bool gpio__get(gpio_s gpio);
void gpio__set(gpio_s gpio);   ///< Sets the pin value as 'high' -> 3.3v
void gpio__reset(gpio_s gpio); ///< Sets the pin value as 'low' -> ground
void gpio__toggle(gpio_s gpio);
