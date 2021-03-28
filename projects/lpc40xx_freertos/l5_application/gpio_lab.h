// file gpio_lab.h
#pragma once
#include "lpc40xx.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t port;
  uint8_t pin;
  int port_pin;
} port_pin_s;

typedef enum {
  LPC_PCLCD = 0, // LCD controller power/clock control bit.  rst = 0
  LPC_PCTIM0,    // Timer/Counter 0 power/clock control bit. rst = 1
  LPC_PCTIM1,    // Timer/Counter 1 power/clock control bit. rst = 1
  LPC_PCUART0,   // UART0 power/clock control bit.           rst = 1
  LPC_PCUART1,   // UART1 power/clock control bit.           rst = 1
  LPC_PCPWM0,    // PWM0 power/clock control bit.            rst = 0
  LPC_PCPWM1,    // PWM1 power/clock control bit.            rst = 0
  LPC_PCI2C0,    // I2C0 interface power/clock control bit.  rst = 1
  LPC_PCUART4,   // UART4 power/clock control bit.           rst = 0
  LPC_PCRTC,     // RTC and Event Monitor/Recorder power/clock control bit.  rst = 1
  LPC_PCSSP1,    // SSP 1 interface power/clock control bit. rst = 0
  LPC_PCEMC,     // External Memory Controller power/clock control bit.      rst = 0
  LPC_PCADC,   // A/D converter (ADC) power/clock control bit.Note: Clear the PDN bit in the AD0CR before clearing this
               // bit, and set this bit before attempting to set PDN. rst = 0
  LPC_PCCAN1,  // CAN Controller 1 power/clock control bit.rst = 0
  LPC_PCCAN2,  // CAN Controller 2 power/clock control bit.rst = 0
  LPC_PCGPIO,  // Power/clock control bit for IOCON, GPIO, and GPIO interrupts. rst = 1
  LPC_PCSPIFI, // SPI Flash Interface power/clock control bit. rst = 0
  LPC_PCMCPWM, // Motor Control PWM power/clock control bit.   rst = 0
  LPC_PCQEI,   // Quadrature Encoder Interface power/clock control bit.  rst = 0
  LPC_PCI2C1,  // I2C1 interface power/clock control bit.  rst = 1
  LPC_PCSSP2,  // SSP2 interface power/clock control bit.  rst = 0
  LPC_PCSSP0,  // SSP0 interface power/clock control bit.  rst = 0
  LPC_PCTIM2,  // Timer 2 power/clock control bit.         rst = 0
  LPC_PCTIM3,  // Timer 3 power/clock control bit.         rst = 0
  LPC_PCUART2, // UART 2 power/clock control bit.          rst = 0
  LPC_PCUART3, // UART 3 power/clock control bit.          rst = 0
  LPC_PCI2C2,  // I2C interface 2 power/clock control bit. rst = 1
  LPC_PCI2S,   // I2S interface power/clock control bit.   rst = 0
  LPC_PCSDC,   // SD Card interface power/clock control bit. rst = 0
  LPC_PCGPDMA, // GPDMA function power/clock control bit.  rst = 0
  LPC_PCENET,  // Ethernet block power/clock control bit.  rst = 0
  LPC_PCUSB,   // USB interface power/clock control bit.   rst = 0
} Peripheral_Symbol_e;

/// Should alter the hardware registers to set the pin as input
void gpiox__set_as_input(port_pin_s pin_num);

/// Should alter the hardware registers to set the pin as output
void gpiox__set_as_output(port_pin_s pin_num);

/// Should alter the hardware registers to set the pin as high
void gpiox__set_high(port_pin_s pin_num);

/// Should alter the hardware registers to set the pin as low
void gpiox__set_low(port_pin_s pin_num);

void gpiox__set(port_pin_s pin_num, bool high);

bool gpiox__get_level(port_pin_s pin_num);

void gpiox__trigger_level(port_pin_s gpio_pin, int posedge_or_negedge);

bool get_pin_INT_status(port_pin_s pin_num, int rising_or_falling);

void Lpc_peripheral_power_control(Peripheral_Symbol_e symbol, int power_level);