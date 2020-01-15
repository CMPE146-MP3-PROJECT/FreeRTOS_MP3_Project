#include <stdint.h>

#include "startup.h"

#include "FreeRTOS.h"
#include "lpc40xx.h"

static void startup__init_data_sram(void);
static void startup__init_bss_sram(void);

void startup__initialize_ram(void) {
  startup__init_data_sram();
  startup__init_bss_sram();
}

/**
 * References:
 * - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0439b/BEHBJHIG.html
 */
void startup__initialize_fpu(void) {
  SCB->CPACR |= (0xF << 20); // Set [23:20]
}

void startup__initialize_interrupts(void) {
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

static void startup__init_data_sram(void) {
  extern void *_bdata_lma;
  extern void *_bdata_vma;
  extern void *_data_end;

  uint8_t *src = (uint8_t *)&_bdata_lma;
  uint8_t *dest = (uint8_t *)&_bdata_vma;
  while (dest < (uint8_t *)&_data_end) {
    *dest = *src;
    dest++;
    src++;
  }
}

static void startup__init_bss_sram(void) {
  extern void *_bss_start;
  extern void *_bss_end;

  uint8_t *ptr = (uint8_t *)&_bss_start;
  while (ptr < (uint8_t *)&_bss_end) {
    *ptr = 0U;
    ptr++;
  }
}
