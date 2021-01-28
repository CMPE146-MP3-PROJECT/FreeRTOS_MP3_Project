#include "gpio.h"

#include "lpc40xx.h"

static const LPC_GPIO_TypeDef *gpio_memory_map[] = {LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3, LPC_GPIO4, LPC_GPIO5};

static LPC_GPIO_TypeDef *gpio__get_struct(gpio_s gpio) { return (LPC_GPIO_TypeDef *)gpio_memory_map[gpio.port_number]; }

static uint32_t gpio__get_pin_mask(gpio_s gpio) { return (UINT32_C(1) << gpio.pin_number); }

static volatile uint32_t *gpio__get_iocon(gpio_s gpio) {
  volatile uint32_t *IOCON_base = (volatile uint32_t *)LPC_IOCON;

  // Each port is offset by thirty-two 32-bit registers
  volatile uint32_t *port_offset = IOCON_base + (32 * gpio.port_number);

  // Each pin configuration is a single 32-bit
  volatile uint32_t *pin_iocon = (port_offset + gpio.pin_number);

  return pin_iocon;
}

/*******************************************************************************
 *
 *                      P U B L I C    F U N C T I O N S
 *
 ******************************************************************************/

gpio_s gpio__construct(gpio__port_e port, uint8_t pin_number_0_to_31) {
  gpio_s gpio = {
      .port_number = (uint8_t)port,
      .pin_number = pin_number_0_to_31,
  };

  return gpio;
}

gpio_s gpio__construct_as_input(gpio__port_e port, uint8_t pin_number_0_to_31) {
  gpio_s gpio = gpio__construct(port, pin_number_0_to_31);
  gpio__set_as_input(gpio);
  return gpio;
}

gpio_s gpio__construct_as_output(gpio__port_e port, uint8_t pin_number_0_to_31) {
  gpio_s gpio = gpio__construct(port, pin_number_0_to_31);
  gpio__set_as_output(gpio);
  return gpio;
}

gpio_s gpio__construct_with_function(gpio__port_e port, uint8_t pin_number_0_to_31, gpio__function_e function) {
  gpio_s gpio = gpio__construct(port, pin_number_0_to_31);
  gpio__set_function(gpio, function);
  return gpio;
}

void gpio__set_function(gpio_s gpio, gpio__function_e function) {
  volatile uint32_t *pin_iocon = gpio__get_iocon(gpio);

  // Reference chapter 7: table 83
  const uint32_t config_mask = UINT32_C(7);
  *pin_iocon &= ~(config_mask << 0);
  *pin_iocon |= ((uint32_t)function & config_mask);
}

void gpio__enable_open_drain(gpio_s gpio) {
  volatile uint32_t *pin_iocon = gpio__get_iocon(gpio);
  *pin_iocon |= (UINT32_C(1) << 10);
}

void gpio__enable_pull_down_resistors(gpio_s gpio) {
  volatile uint32_t *pin_iocon = gpio__get_iocon(gpio);

  const uint32_t mode_mask = UINT32_C(3);
  *pin_iocon &= ~(mode_mask << 3);
  *pin_iocon |= (UINT32_C(1) << 3);
}

void gpio__set_as_input(gpio_s gpio) { gpio__get_struct(gpio)->DIR &= ~gpio__get_pin_mask(gpio); }

void gpio__set_as_output(gpio_s gpio) { gpio__get_struct(gpio)->DIR |= gpio__get_pin_mask(gpio); }

bool gpio__get(gpio_s gpio) { return (gpio__get_struct(gpio)->PIN & gpio__get_pin_mask(gpio)); }

void gpio__set(gpio_s gpio) { gpio__get_struct(gpio)->SET = gpio__get_pin_mask(gpio); }

void gpio__reset(gpio_s gpio) { gpio__get_struct(gpio)->CLR = gpio__get_pin_mask(gpio); }

void gpio__toggle(gpio_s gpio) {
  // Avoiding XOR logic with PIN register to make it thread safe
  if (gpio__get(gpio)) {
    gpio__reset(gpio);
  } else {
    gpio__set(gpio);
  }
}
