#include "startup.h"

#include "FreeRTOS.h"
#include "lpc40xx.h"
#include "ram.h"

static void startup__initialize_ram(void);
static void startup__initialize_fpu(void);
static void startup__initialize_interrupts(void);

void startup__initialize(void) {
  startup__initialize_ram();
  startup__initialize_fpu();
  startup__initialize_interrupts();
}

static void startup__initialize_ram(void) {
  ram__init_data();
  ram__init_bss();
}

/**
 * References:
 * - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0439b/BEHBJHIG.html
 */
static void startup__initialize_fpu(void) {
  SCB->CPACR |= (0xF << 20); // Set [23:20]
}

static void startup__initialize_interrupts(void) {
  /* All peripherals should set their priority lower than (which is higher number)
   * configMAX_SYSCALL_INTERRUPT_PRIORITY set at FreeRTOSConfig.h
   *
   * Should be the same as (configMAX_SYSCALL_INTERRUPT_PRIORITY >> configPRIO_BITS_NOT_IMPLEMENTED) + 1;
   *
   * Reference: https: // www.freertos.org/RTOS-Cortex-M3-M4.html
   */
  const int peripheral_interrupt_priority = RTOS_HIGHEST_INTERRUPT_PRIORITY + 1;

  const int first_peripheral = (int)WDT_IRQn;
  const int last_peripheral = first_peripheral + (int)EEPROM_IRQn;

  for (int peripheral = first_peripheral; peripheral <= last_peripheral; peripheral++) {
    NVIC_SetPriority(peripheral, peripheral_interrupt_priority);
  }

  /* Note 1: enabling of IRQ or FIQ is not needed because interrupts are enabled so we can use them without FreeRTOS
   * Note 2: We should not alter base priority register as the FreeRTOS API will alter it.
   *
   * Leaving dead code here for reference:
   * __set_BASEPRI(0xff);
   * __enable_irq();
   * __enable_fault_irq();
   */
}
