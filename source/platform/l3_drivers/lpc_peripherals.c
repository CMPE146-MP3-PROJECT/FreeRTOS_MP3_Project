#include "lpc_peripherals.h"

#include "lpc40xx.h"

/**
 * The enumeration values correspond to the peripherals' power-on bit
 * Check UM10562.pdf, Chapter 3: System and clock control
 */
static const uint8_t lpc_peripheral_pconp_bit_map[] = {
    [LPC_PERIPHERAL__UART0] = 3,
    [LPC_PERIPHERAL__UART1] = 4,
    [LPC_PERIPHERAL__UART2] = 24,
    [LPC_PERIPHERAL__UART3] = 25,
};

/// An unregistered interrupt handler is a forever loop
static void lpc_peripheral__halt_handler(void) {
  while (1) {
  }
}

/**
 * Map of the user ISR registrations
 * These will call the halt handler unless user changes their ISR slot by calling lpc_peripheral__enable_interrupt()
 */
static void lpc_peripheral__isr_callback_f lpc_peripheral__isr_registrations[32 + 9] = {
  lpc_peripheral__halt_handler, // 16 WDT
  lpc_peripheral__halt_handler, // 17 Timer 0
  lpc_peripheral__halt_handler, // 18 Timer 1
  lpc_peripheral__halt_handler, // 19 Timer 2
  lpc_peripheral__halt_handler, // 20 Timer 3
  lpc_peripheral__halt_handler, // 21 UART 0
  lpc_peripheral__halt_handler, // 22 UART 1
  lpc_peripheral__halt_handler, // 23 UART 2

  lpc_peripheral__halt_handler, // 24 UART 3
  lpc_peripheral__halt_handler, // 25 PWM 1
  lpc_peripheral__halt_handler, // 26 I2C 0
  lpc_peripheral__halt_handler, // 27 I2C 1
  lpc_peripheral__halt_handler, // 28 I2C 2
  lpc_peripheral__halt_handler, // 29 UNUSED
  lpc_peripheral__halt_handler, // 30 SSP 0
  lpc_peripheral__halt_handler, // 31 SSP 1

  lpc_peripheral__halt_handler, // 32 PLL 0
  lpc_peripheral__halt_handler, // 33 RTC and Event Monitor/Recorder
  lpc_peripheral__halt_handler, // 34 External Interrupt 0 (EINT 0)
  lpc_peripheral__halt_handler, // 35 External Interrupt 1 (EINT 1)
  lpc_peripheral__halt_handler, // 36 External Interrupt 2 (EINT 2)
  lpc_peripheral__halt_handler, // 37 External Interrupt 3 (EINT 3)
  lpc_peripheral__halt_handler, // 38 ADC
  lpc_peripheral__halt_handler, // 39 BOD

  lpc_peripheral__halt_handler, // 40 USB
  lpc_peripheral__halt_handler, // 41 CAN
  lpc_peripheral__halt_handler, // 42 DMA Controller
  lpc_peripheral__halt_handler, // 43 I2S
  lpc_peripheral__halt_handler, // 44 Ethernet
  lpc_peripheral__halt_handler, // 45 SD Card Interface
  lpc_peripheral__halt_handler, // 46 Motor Control PWM
  lpc_peripheral__halt_handler, // 47 PLL 1

  lpc_peripheral__halt_handler, // 48 Quadrature Encoder
  lpc_peripheral__halt_handler, // 49 USB Activity
  lpc_peripheral__halt_handler, // 50 CAN Activity
  lpc_peripheral__halt_handler, // 51 UART 4
  lpc_peripheral__halt_handler, // 52 SSP 2
  lpc_peripheral__halt_handler, // 53 LCD
  lpc_peripheral__halt_handler, // 54 GPIO Interrupt
  lpc_peripheral__halt_handler, // 55 PWM 0

  lpc_peripheral__halt_handler, // 56 EEPROM
}

/**
 * This function is intentionally not declared at the header file
 * This is registered by the startup code and registered as the interrupt callback for each peripheral
 */
void lpc_peripheral__interrupt_dispatcher(void) {
  /* Get the IRQ number we are in.  Note that ICSR's real ISR bits are offset by 16.
   * We can read ICSR register too, but let's just read 8-bits directly.
   */
  const uint8_t isr_num = (*((uint8_t *)0xE000ED04)) - 16; // (SCB->ICSR & 0xFF) - 16;

  /* Lookup the function pointer we want to call and make the call */
  lpc_peripheral__isr_callback_f isr_to_service = lpc_peripheral__isr_registrations[isr_num];
  isr_to_service();
}

void lpc_peripheral__turn_on_power_to(lpc_peripherals_e peripheral) {
  if (peripheral < sizeof(lpc_peripheral_pconp_bit_map)) {
    const uint32_t power_on_bit = (uint32_t)lpc_peripheral_pconp_bit_map[peripheral];
    LPC_SC->PCONP |= (UINT32_C(1) << power_on_bit);
  }
}

void lpc_peripheral__enable_interrupt(lpc_peripherals_e peripheral, lpc_peripheral__isr_callback_f isr_callback) {
  lpc_peripheral__isr_registrations[peripheral] = isr_callback;
  NVIC_EnableIRQ(peripheral); // Use CMS API
}
