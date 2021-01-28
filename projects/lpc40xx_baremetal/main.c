#include "gpio.h"
#include "startup.h"

static void delay(void);
static void halt(void);

/**
 * On a 'non-hosted environment', we actually do not really need the 'main' function.
 * We control exactly where CPU begins its first instructions at interrupt_vector_table[] array below.
 *
 * Note:
 * Heap memory facility is not setup. If you use 'malloc()' or uses an API that does, such as FreeRTOS API
 * then you would need to provide your own glue code for malloc() to invoke _sbrk() function mentioned below.
 * You are welcome to reference sbrk.c at the lpc40xx_freertos project
 */
int main(void) {
  /* This has to be done otherwise initial values of RAM variables will not be setup correctly
   * Remember that *data* section needs to be copied from Flash to RAM to initialize global variables
   */
  startup__initialize_ram();

  // led0 is on the stack memory, hence CPU SP needs to be setup
  // correctly otherwise we will not be able to execute this code
  gpio_s led0 = gpio__construct_as_output(GPIO__PORT_2, 3);

  while (1) {
    gpio__set(led0);
    delay();

    gpio__reset(led0);
    delay();
  }

  return 0;
}

static void delay(void) {
  for (int i = 0; i < 500 * 1000; i++) {
    __asm__("NOP");
  }
}

static void halt(void) {
  while (1) {
  }
}

/* These 'Platform Glue' functions may be needed if you use malloc() or standard output (printf)
 * Please uncomment, and provide your own implementation; @see libc.pdf
 */
#if 0
int _write() { return 0; }
int _read() { return 0; }

int _fstat() { return 0; }
int _isatty() { return 0; }
int _lseek() { return 0; }
int _close() { return 0; }

int _sbrk() { return 0; }
#endif

/* The linker script: layout_lpc4078.ld informs of the stack memory
 * This is named as '_estack' for consistency with GNU
 */
extern void *_estack;

// Function pointer type for void function
typedef void (*function__void_f)(void);

/**
 * ARM processor starts the executing by doing this in Verilog code
 *
 * // Flash memory address of LPC40xx is at zero
 * const uint32_t *flash_memory = 0x00000000;
 *
 * // Before a C function can be called, the SP of CPU needs correct value
 * SP (stack pointer) = flash_memory[0];
 *
 * // Fetch the 'Reset Interrupt' of ARM Cortex-M4, and jump to that location
 * // 'Reset Interrupt' is the boot location
 * PC = flash_memory[1];
 */
__attribute__((section(".interrupt_vector_table"))) const function__void_f interrupt_vector_table[] = {
    (function__void_f)&_estack, // 0 ARM: Initial stack pointer
    (function__void_f)main,     // 1 ARM: Initial program counter; your board will explode if you change this

    /* Note that we only have 16 interrupt vectors, and not the NXP LPC40xx peripheral interrupts
     * This is just done for demonstration of absolute minimal bare-metal project which does not
     * utilize any peripheral interrupts
     */
    halt, // 2 ARM: Non-maskable interrupt
    halt, // 3 ARM: Hard fault
    halt, // 4 ARM: Memory management fault
    halt, // 5 ARM: Bus fault
    halt, // 6 ARM: Usage fault
    halt, // 7 ARM: Reserved
    halt, // 8 ARM: Reserved
    halt, // 9 ARM: Reserved
    halt, // 10 ARM: Reserved
    halt, // 11 ARM: Supervisor call (SVCall)
    halt, // 12 ARM: Debug monitor
    halt, // 13 ARM: Reserved
    halt, // 14 ARM: Pendable request for system service (PendableSrvReq)
    halt, // 15 ARM: System Tick Timer (SysTick)
};
