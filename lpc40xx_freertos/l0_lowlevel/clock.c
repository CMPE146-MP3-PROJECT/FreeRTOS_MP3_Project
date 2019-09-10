#include "clock.h"

/**
 * FreeRTOS 'configCPU_CLOCK_HZ' references this code to get the CPU frequency.
 * Then, the Cortex-M4 port uses that reference to configure the RTOS tick rate.
 *
 * We use fixed CPU frequency because startup.h configures fixed CPU frequency, so
 * unfortunately startup.c implementation needs to match this number but it is rare
 * that anyone needs to change this (ever).
 */
uint32_t clock__get_core_clock_hz(void) { return (96UL * 1000 * 1000); }
