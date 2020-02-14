#include <stdbool.h>
#include <stdio.h>

#include "crash.h"
#include "freertos_interrupt_handlers.h"
#include "function_types.h"
#include "lpc40xx.h"

/**
 * _estack symbol is actually a pointer to the start of the stack memory (provided by the linker script).
 * Declaring as unsigned int to inform compiler that this symbol is constant and defined at link time.
 * To get value, reference &_estack.
 */
extern void *_estack;

/**
 * @{
 * These functions are from entry_point.c and lpc_peripherals.c but they are not
 * declared in the header file to sort of hide them from the public API
 */
extern void entry_point(void);
extern void lpc_peripheral__interrupt_dispatcher(void);
/** @} */

static void halt(void);
static void isr_hard_fault(void);

/**
 * This is non static otherwise compiler optimizes this function away but it is used in assembly code
 * Alternative is to use '__attribute__((used))' attribute prior to isr_hard_fault_handler()
 */
void isr_hard_fault_handler(unsigned long *hardfault_args);

__attribute__((section(".interrupt_vector_table"))) const function__void_f interrupt_vector_table[] = {
    /**
     * Core interrupt vectors
     */
    (function__void_f)&_estack,  // 0 ARM: Initial stack pointer
    entry_point,                 // 1 ARM: Initial program counter; your board will explode if you change this
    halt,                        // 2 ARM: Non-maskable interrupt
    isr_hard_fault,              // 3 ARM: Hard fault
    halt,                        // 4 ARM: Memory management fault
    halt,                        // 5 ARM: Bus fault
    halt,                        // 6 ARM: Usage fault
    halt,                        // 7 ARM: Reserved
    halt,                        // 8 ARM: Reserved
    halt,                        // 9 ARM: Reserved
    halt,                        // 10 ARM: Reserved
    vPortSVCHandler_wrapper,     // 11 ARM: Supervisor call (SVCall)
    halt,                        // 12 ARM: Debug monitor
    halt,                        // 13 ARM: Reserved
    xPortPendSVHandler_wrapper,  // 14 ARM: Pendable request for system service (PendableSrvReq)
    xPortSysTickHandler_wrapper, // 15 ARM: System Tick Timer (SysTick)

    /**
     * Device interrupt vectors
     */
    lpc_peripheral__interrupt_dispatcher, // 16 WDT
    lpc_peripheral__interrupt_dispatcher, // 17 Timer 0
    lpc_peripheral__interrupt_dispatcher, // 18 Timer 1
    lpc_peripheral__interrupt_dispatcher, // 19 Timer 2
    lpc_peripheral__interrupt_dispatcher, // 20 Timer 3
    lpc_peripheral__interrupt_dispatcher, // 21 UART 0
    lpc_peripheral__interrupt_dispatcher, // 22 UART 1
    lpc_peripheral__interrupt_dispatcher, // 23 UART 2
    lpc_peripheral__interrupt_dispatcher, // 24 UART 3
    lpc_peripheral__interrupt_dispatcher, // 25 PWM 1
    lpc_peripheral__interrupt_dispatcher, // 26 I2C 0
    lpc_peripheral__interrupt_dispatcher, // 27 I2C 1
    lpc_peripheral__interrupt_dispatcher, // 28 I2C 2
    lpc_peripheral__interrupt_dispatcher, // 29 UNUSED
    lpc_peripheral__interrupt_dispatcher, // 30 SSP 0
    lpc_peripheral__interrupt_dispatcher, // 31 SSP 1
    lpc_peripheral__interrupt_dispatcher, // 32 PLL 0
    lpc_peripheral__interrupt_dispatcher, // 33 RTC and Event Monitor/Recorder
    lpc_peripheral__interrupt_dispatcher, // 34 External Interrupt 0 (EINT 0)
    lpc_peripheral__interrupt_dispatcher, // 35 External Interrupt 1 (EINT 1)
    lpc_peripheral__interrupt_dispatcher, // 36 External Interrupt 2 (EINT 2)
    lpc_peripheral__interrupt_dispatcher, // 37 External Interrupt 3 (EINT 3)
    lpc_peripheral__interrupt_dispatcher, // 38 ADC
    lpc_peripheral__interrupt_dispatcher, // 39 BOD
    lpc_peripheral__interrupt_dispatcher, // 40 USB
    lpc_peripheral__interrupt_dispatcher, // 41 CAN
    lpc_peripheral__interrupt_dispatcher, // 42 DMA Controller
    lpc_peripheral__interrupt_dispatcher, // 43 I2S
    lpc_peripheral__interrupt_dispatcher, // 44 Ethernet
    lpc_peripheral__interrupt_dispatcher, // 45 SD Card Interface
    lpc_peripheral__interrupt_dispatcher, // 46 Motor Control PWM
    lpc_peripheral__interrupt_dispatcher, // 47 PLL 1
    lpc_peripheral__interrupt_dispatcher, // 48 Quadrature Encoder
    lpc_peripheral__interrupt_dispatcher, // 49 USB Activity
    lpc_peripheral__interrupt_dispatcher, // 50 CAN Activity
    lpc_peripheral__interrupt_dispatcher, // 51 UART 4
    lpc_peripheral__interrupt_dispatcher, // 52 SSP 2
    lpc_peripheral__interrupt_dispatcher, // 53 LCD
    lpc_peripheral__interrupt_dispatcher, // 54 GPIO Interrupt
    lpc_peripheral__interrupt_dispatcher, // 55 PWM 0
    lpc_peripheral__interrupt_dispatcher, // 56 EEPROM
};

static void halt(void) {
  // This statement resolves compiler warning: variable define but not used
  (void)interrupt_vector_table;

  const unsigned isr_num = (*((uint8_t *)0xE000ED04));
  fprintf(stderr, "Unexpected CPU exception ");
  fprintf(stderr, "%u (interrupt) has occured and the program will now halt\n", isr_num);

  if (isr_num < 16) {
    static const char *table[] = {"estack",      "reset",    "NMI",      "hard fault", "memory fault", "bus fault",
                                  "usage fault", "reserved", "reserved", "reserved",   "reserved",     "rtos",
                                  "debug",       "reserved", "rtos",     "rtos"};
    fprintf(stderr, "Exception appears to be '%s'\n", table[isr_num]);
  } else {
    fprintf(stderr, "Did you register the interrupt correctly using lpc_peripherals.h API?");
  }

  while (true) {
    ;
  }
}

/**
 * Explanation of register usage dictated by ARM's EABI:
 * https://stackoverflow.com/questions/261419/what-registers-to-save-in-the-arm-c-calling-convention
 */
static void isr_hard_fault(void) {
  __asm__ volatile("tst lr, #4                                    \n"
                   "ite eq                                        \n"
                   "mrseq r0, msp                                 \n"
                   "mrsne r0, psp                                 \n"
                   "ldr r1, [r0, #24]                             \n"
                   "ldr r2, handler_address                       \n"
                   "bx r2                                         \n"
                   "handler_address: .word isr_hard_fault_handler \n");
}

void isr_hard_fault_handler(unsigned long *hardfault_args) {
  crash__registers_s *c = crash__record_get();

  c->registers[0] = ((unsigned long)hardfault_args[0]);
  c->registers[1] = ((unsigned long)hardfault_args[1]);
  c->registers[2] = ((unsigned long)hardfault_args[2]);
  c->registers[3] = ((unsigned long)hardfault_args[3]);

  c->r12 = ((unsigned long)hardfault_args[4]);
  c->lr = ((unsigned long)hardfault_args[5]) - 1;
  c->pc = ((unsigned long)hardfault_args[6]);
  c->psr = ((unsigned long)hardfault_args[7]);

  // Save the data and reboot and print the registers upon the next boot
  NVIC_SystemReset();
}
