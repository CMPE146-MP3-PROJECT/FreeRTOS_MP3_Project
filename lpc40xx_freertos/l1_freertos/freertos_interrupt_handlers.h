/**
 * @file
 * FreeRTOS functions that must be hooked into the core interrupt vector table.
 * These are named as "_wrapper" because we wrap the real FreeRTOS port.c functions
 * in a small wrapper to measure the run-time statistics
 */
#pragma once

/**
 * Supervisor Call - Executes scheduler; must be executed in privileged mode.
 */
void vPortSVCHandler_wrapper(void);

/**
 * Pend Supervisor Call - Acts like a service request for the supervisor;
 * intent is to avoid preempting in interrupt context.
 */
void xPortPendSVHandler_wrapper(void);

/**
 * System Tick - To be called every OS tick; this is an entry point to the scheduler.
 */
void xPortSysTickHandler_wrapper(void);
