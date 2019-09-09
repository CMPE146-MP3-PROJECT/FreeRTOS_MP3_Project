#include <stdbool.h>
#include <stdint.h>

#include "freertos_hooks.h"

/**
 * _estack symbol is actually a pointer to the stack pointer value provided by the linker script.
 * Declaring as unsigned int to inform compiler that this symbol is constant and defined at link time.
 * To get value, reference &_estack.
 */
extern unsigned int _estack;
extern void cpu_startup_entry_point(void);

static void halt(void);

typedef void (*void_func_ptr_t)(void);

__attribute__ ((section(".interrupt_vector_table")))
void_func_ptr_t interrupt_vector_table[] = {
    /**
     * Core interrupt vectors
     */
    (void_func_ptr_t)&_estack,  // 0 ARM: Initial stack pointer
    cpu_startup_entry_point,  // 1 ARM: Initial program counter
    halt,  // 2 ARM: Non-maskable interrupt
    halt,  // 3 ARM: Hard fault
    halt,  // 4 ARM: Memory management fault
    halt,  // 5 ARM: Bus fault
    halt,  // 6 ARM: Usage fault
    halt,  // 7 ARM: Reserved
    halt,  // 8 ARM: Reserved
    halt,  // 9 ARM: Reserved
    halt,  // 10 ARM: Reserved
    vPortSVCHandler,  // 11 ARM: Supervisor call (SVCall)
    halt,  // 12 ARM: Debug monitor
    halt,  // 13 ARM: Reserved
    xPortPendSVHandler, // 14 ARM: Pendable request for system service (PendableSrvReq)
    xPortSysTickHandler, // 15 ARM: System Tick Timer (SysTick)

    /**
     * Device interrupt vectors
     */
    halt, // 16 WDT
    halt, // 17 Timer 0
    halt, // 18 Timer 1
    halt, // 19 Timer 2
    halt, // 20 Timer 3
    halt, // 21 UART 0
    halt, // 22 UART 1
    halt, // 23 UART 2
    halt, // 24 UART 3
    halt, // 25 PWM 1
    halt, // 26 I2C 0
    halt, // 27 I2C 1
    halt, // 28 I2C 2
    halt, // 29 UNUSED
    halt, // 30 SSP 0
    halt, // 31 SSP 1
    halt, // 32 PLL 0
    halt, // 33 RTC and Event Monitor/Recorder
    halt, // 34 External Interrupt 0 (EINT 0)
    halt, // 35 External Interrupt 1 (EINT 1)
    halt, // 36 External Interrupt 2 (EINT 2)
    halt, // 37 External Interrupt 3 (EINT 3)
    halt, // 38 ADC
    halt, // 39 BOD
    halt, // 40 USB
    halt, // 41 CAN
    halt, // 42 DMA Controller
    halt, // 43 I2S
    halt, // 44 Ethernet
    halt, // 45 SD Card Interface
    halt, // 46 Motor Control PWM
    halt, // 47 PLL 1
    halt, // 48 Quadrature Encoder
    halt, // 49 USB Activity
    halt, // 50 CAN Activity
    halt, // 51 UART 4
    halt, // 52 SSP 2
    halt, // 53 LCD
    halt, // 54 GPIO Interrupt
    halt, // 55 PWM 0
    halt, // 56 EEPROM
};

static void halt(void)
{
    // This statement resolves compiler warning: variable define but not used
    (void)interrupt_vector_table;

    while (true) {}
}
