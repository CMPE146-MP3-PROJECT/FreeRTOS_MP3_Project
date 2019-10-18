#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * Data structure that is saved in "persistent ram" region when a crash occurs
 */
typedef struct {
  uint32_t signature;
  uint32_t signature_inverted;

  uint32_t registers[4]; // R0-R3
  uint32_t pc, lr, psr, r12;
} crash__registers_s;

/**
 * Records the crash signature, and returns the struct pointer to populate further members
 * It is best to not print out the data during the crash, so this merely "records" it and then upon the next
 * boot, crash__report_if_occurred() should be used to print the crash report when the system is healthy
 */
crash__registers_s *crash__record_get(void);

/**
 * Reports the crash information (if any) and clears it
 */
bool crash__report_if_occurred(void);
