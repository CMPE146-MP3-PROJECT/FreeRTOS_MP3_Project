#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "freertos_interrupt_handlers.h"
#include "function_types.h"

/**
 * _estack symbol is actually a pointer to the start of the stack memory (provided by the linker script).
 * Declaring as unsigned int to inform compiler that this symbol is constant and defined at link time.
 * To get value, reference &_estack.
 */
extern void *_estack;

/**
 * @{
 * These functions are from entry_point.c and lpc_peripherals.c but they are not declared in the header
 * file to sort of hide them from the public API
 */
extern void entry_point(void);
extern void lpc_peripheral__interrupt_dispatcher(void);
/** @} */

static void halt(void);

__attribute__((section(".interrupt_vector_table"))) const function__void_f interrupt_vector_table[] = {
    /**
     * Core interrupt vectors
     */
    (function__void_f)&_estack, // 0 ARM: Initial stack pointer
    entry_point,                // 1 ARM: Initial program counter; your board will explode if you change this
    halt,                       // 2 ARM: Non-maskable interrupt
    halt,                       // 3 ARM: Hard fault
    halt,                       // 4 ARM: Memory management fault
    halt,                       // 5 ARM: Bus fault
    halt,                       // 6 ARM: Usage fault
    halt,                       // 7 ARM: Reserved
    halt,                       // 8 ARM: Reserved
    halt,                       // 9 ARM: Reserved
    halt,                       // 10 ARM: Reserved
    vPortSVCHandler,            // 11 ARM: Supervisor call (SVCall)
    halt,                       // 12 ARM: Debug monitor
    halt,                       // 13 ARM: Reserved
    xPortPendSVHandler,         // 14 ARM: Pendable request for system service (PendableSrvReq)
    xPortSysTickHandler,        // 15 ARM: System Tick Timer (SysTick)

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

  fprintf(stderr, "CPU exception has occured and the program will now halt\n");
  while (true) {
    ;
  }
}
