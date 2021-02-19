// @file gpio_isr.h
#include "gpio_lab.h"
//#pragma once

typedef enum {
  GPIO_INTR__FALLING_EDGE,
  GPIO_INTR__RISING_EDGE,
} gpio_interrupt_e;

typedef struct {
  port_pin_s port_pin;
  int trigger;
} port_pin_trigger;

// Function pointer type (demonstrated later in the code sample)
typedef void (*function_pointer_t)(void);

// Allow the user to attach their callbacks
void gpiox__attach_interrupt(port_pin_s pin, gpio_interrupt_e interrupt_type, function_pointer_t callback);

// Our main() should configure interrupts to invoke this dispatcher where we will invoke user attached callbacks
// You can hijack 'interrupt_vector_table.c' or use API at lpc_peripherals.h
void gpiox__interrupt_dispatcher(void);

port_pin_trigger check_interrupt_pin();