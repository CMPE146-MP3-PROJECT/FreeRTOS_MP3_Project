#include "lpc_peripherals.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lpc40xx.h"

// clang-format off
/**
 * The enumeration values correspond to the peripherals' power-on bit
 * Check UM10562.pdf, Chapter 3: System and clock control
 */
static const uint8_t lpc_peripheral_pconp_bit_map[] = {
    [LPC_PERIPHERAL__TIMER0] = 1,
    [LPC_PERIPHERAL__TIMER1] = 2,
    [LPC_PERIPHERAL__TIMER2] = 22,
    [LPC_PERIPHERAL__TIMER3] = 23,

    [LPC_PERIPHERAL__UART0] = 3,
    [LPC_PERIPHERAL__UART1] = 4,
    [LPC_PERIPHERAL__UART2] = 24,
    [LPC_PERIPHERAL__UART3] = 25,

    [LPC_PERIPHERAL__I2C0] = 7,
    [LPC_PERIPHERAL__I2C1] = 19,
    [LPC_PERIPHERAL__I2C2] = 26,

    [LPC_PERIPHERAL__SSP0] = 21,
    [LPC_PERIPHERAL__SSP1] = 10,
    [LPC_PERIPHERAL__SSP2] = 20,

    [LPC_PERIPHERAL__PWM1] = 6,
    [LPC_PERIPHERAL__ADC] = 12,
    [LPC_PERIPHERAL__GPDMA] = 29,
};
// clang-format on

/// An unregistered interrupt handler is a forever loop
static void lpc_peripheral__halt_handler(void) {
  while (1) {
  }
}

/**
 * Map of the user ISR registrations.
 * This is our own memory map that is used by lpc_peripheral__interrupt_dispatcher() and it is not the real ISR vector
 * The real ISR vector registers lpc_peripheral__interrupt_dispatcher() for all peripheral ISR and we dispatch user
 * registered ISR here.
 *
 * These will call the halt handler unless user changes their ISR slot by calling lpc_peripheral__enable_interrupt()
 */
static function__void_f lpc_peripheral__isr_registrations[32 + 9] = {
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
};

/**
 * This function is intentionally not declared at the header file
 * This is registered by the startup code and registered as the interrupt callback for each peripheral
 */
void lpc_peripheral__interrupt_dispatcher(void) {
  vRunTimeStatIsrEntry();

  /* Get the IRQ number we are in.  Note that ICSR's real ISR bits are offset by 16.
   * We can read ICSR register too, but let's just read 8-bits directly.
   */
  const uint8_t isr_num = (*((uint8_t *)0xE000ED04)) - 16; // (SCB->ICSR & 0xFF) - 16;

  /* Lookup the function pointer we want to call and make the call */
  function__void_f isr_to_service = lpc_peripheral__isr_registrations[isr_num];
  isr_to_service();

  // http://www.keil.com/support/docs/3928.htm
  static volatile int memory_write_to_avoid_spurious_interrupt;
  memory_write_to_avoid_spurious_interrupt = 0;
  (void)memory_write_to_avoid_spurious_interrupt; // Avoid 'variable set but not used' warning

  vRunTimeStatIsrExit();
}

void lpc_peripheral__turn_on_power_to(lpc_peripheral_e peripheral) {
  if (peripheral < sizeof(lpc_peripheral_pconp_bit_map)) {
    const uint32_t power_on_bit = (uint32_t)lpc_peripheral_pconp_bit_map[peripheral];
    LPC_SC->PCONP |= (UINT32_C(1) << power_on_bit);
  }
}

bool lpc_peripheral__is_powered_on(lpc_peripheral_e peripheral) {
  bool powered_on = false;

  if (peripheral < sizeof(lpc_peripheral_pconp_bit_map)) {
    const uint32_t power_on_bit = (uint32_t)lpc_peripheral_pconp_bit_map[peripheral];
    powered_on = 0 != (LPC_SC->PCONP & (UINT32_C(1) << power_on_bit));
  }

  return powered_on;
}

void lpc_peripheral__enable_interrupt(lpc_peripheral_e peripheral, function__void_f isr_callback) {
  lpc_peripheral__isr_registrations[peripheral] = isr_callback;

  /**
   * @note:
   * lpc_peripheral_e should match IRQn_Type; we are not exposing NXP header file in our lpc_peripherals.h
   * and therefore we create a mirror image of this enumeration.
   *
   * @warning
   * startup.c sets up interrupt priorities of all peripherals which needs to be at or lower priority than
   * RTOS_HIGHEST_INTERRUPT_PRIORITY (configMAX_SYSCALL_INTERRUPT_PRIORITY). Make sure you DO NOT set the
   * priority higher than the RTOS interrupt (note that higher priority means lower number)
   *
   * Unless you really know what you are doing, never call NVIC_SetPriority() with a priority other than:
   *    NVIC_SetPriority(peripheral, RTOS_HIGHEST_INTERRUPT_PRIORITY + 1);
   */
  const IRQn_Type irq_type = (IRQn_Type)peripheral;
  NVIC_EnableIRQ(irq_type); // Use CMS API
}
