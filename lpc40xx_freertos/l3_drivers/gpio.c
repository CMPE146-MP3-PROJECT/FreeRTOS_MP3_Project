#include "gpio.h"

#include "lpc40xx.h"

static const LPC_GPIO_TypeDef *gpio_memory_map[] = {LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3, LPC_GPIO4};

static LPC_GPIO_TypeDef *gpio__get_struct(gpio_s gpio) { return (LPC_GPIO_TypeDef *)gpio_memory_map[gpio.port_number]; }

static uint32_t gpio__get_pin_mask(gpio_s gpio) { return (UINT32_C(1) << gpio.pin_number); }

/*******************************************************************************
 *
 *                      P U B L I C    F U N C T I O N S
 *
 ******************************************************************************/

gpio_s gpio__instantiate(gpio__port_e port, uint8_t pin_number_0_to_31) {
  gpio_s gpio = {
      .port_number = (uint8_t)port,
      .pin_number = pin_number_0_to_31,
  };

  return gpio;
}

void gpio__set_function(gpio_s gpio, gpio__function_e function) {
  volatile uint32_t *IOCON_base = (volatile uint32_t *)LPC_IOCON;

  // Each port is offset by thirty-two 32-bit registers
  volatile uint32_t *port_offset = IOCON_base + (32 * gpio.port_number);

  // Each pin configuration is a single 32-bit
  volatile uint32_t *pin_offset = (port_offset + gpio.pin_number);

  // This entire 32-bit register is configuration of this single port pin
  volatile uint32_t *pin_config = pin_offset;

  // Reference chapter 7: table 83
  const uint32_t config_mask = UINT32_C(3);
  *pin_config &= ~(config_mask << 0);
  *pin_config |= ((uint32_t)function & config_mask);
}

void gpio__set_as_input(gpio_s gpio) { gpio__get_struct(gpio)->DIR &= ~gpio__get_pin_mask(gpio); }

void gpio__set_as_output(gpio_s gpio) { gpio__get_struct(gpio)->DIR |= gpio__get_pin_mask(gpio); }

bool gpio__get(gpio_s gpio) { return (gpio__get_struct(gpio)->PIN & gpio__get_pin_mask(gpio)); }
void gpio__set(gpio_s gpio) { gpio__get_struct(gpio)->SET = gpio__get_pin_mask(gpio); }
void gpio__reset(gpio_s gpio) { gpio__get_struct(gpio)->CLR = gpio__get_pin_mask(gpio); }

void gpio__toggle(gpio_s gpio) {
  // Avoiding XOR logic to make it thread safe
  if (gpio__get(gpio)) {
    gpio__reset(gpio);
  } else {
    gpio__set(gpio);
  }
}
