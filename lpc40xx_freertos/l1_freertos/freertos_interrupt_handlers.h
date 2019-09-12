#pragma once

/**
 * FreeRTOS functions that must be hooked into the core interrupt vector table.
 */

/**
 * Supervisor Call - Executes scheduler; must be executed in privileged mode.
 */
void vPortSVCHandler(void);

/**
 * Pend Supervisor Call - Acts like a service request for the supervisor;
 * intent is to avoid preempting in interrupt context.
 */
void xPortPendSVHandler(void);

/**
 * System Tick - To be called every OS tick; this is an entry point to the scheduler.
 */
void xPortSysTickHandler(void);
