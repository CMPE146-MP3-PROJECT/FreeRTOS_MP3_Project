#pragma once

/// Resets CPU counters of each task
void vTaskResetRunTimeStats(void);

/**
 * Freezes the CPU counters for the current task
 * Invoke this upon entry to an ISR
 */
void vRunTimeStatIsrEntry(void);

/**
 * Resumes the CPU counters for the current task
 * Invoke this upon exit of an ISR
 */
void vRunTimeStatIsrExit(void);