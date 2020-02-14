#include <stdio.h>

#include "crash.h"

/**
 * Keep this RAM data at a specific location that is not wiped out on startup
 * Note: This section is not part of the *data or *bss section
 */
__attribute__((section(".persistent_ram"))) static crash__registers_s crash_registers;

static const uint32_t crash__signature = 0xDEADBEEF;

static void crash__clear_record(void);

crash__registers_s *crash__record_get(void) {
  // Record the crash signature first before returning the struct
  crash_registers.signature = crash__signature;
  crash_registers.signature_inverted = ~crash__signature;

  return &crash_registers;
}

bool crash__report_if_occurred(void) {
  bool crash_occured = false;

  if (crash__signature == crash_registers.signature && ~crash__signature == crash_registers.signature_inverted) {
    crash_occured = true;

    fprintf(stderr, "----------------------------------------\n"
                    "---- WARNING: We crashed previously ----\n");

    fprintf(stderr, "  PC: 0x%08lX\n", crash_registers.pc);
    fprintf(stderr, "  LR: 0x%08lX\n", crash_registers.lr);
    fprintf(stderr, " PSR: 0x%08lX\n", crash_registers.psr);

    fprintf(stderr, "  R0: 0x%08lX\n", crash_registers.registers[0]);
    fprintf(stderr, "  R1: 0x%08lX\n", crash_registers.registers[1]);
    fprintf(stderr, "  R2: 0x%08lX\n", crash_registers.registers[2]);
    fprintf(stderr, "  R3: 0x%08lX\n", crash_registers.registers[3]);

    fprintf(stderr, "\n"
                    "  Find the *.lst file in your _build directory, and lookup\n"
                    "  the PC value to locate which function caused this.\n"
                    "  The LR should be the 'previous' function that was running.\n"
                    "  R0-R3 are parameters to functions.\n");
  }

  crash__clear_record();
  return crash_occured;
}

static void crash__clear_record(void) {
  crash_registers.signature = 0;
  crash_registers.signature_inverted = 0;
}