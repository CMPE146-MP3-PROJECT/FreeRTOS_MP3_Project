#include "freertos_interrupt_handlers.h"

#include "freertos_runtime_stats.h"
#include "function_types.h"

/**
 * @{
 * @name declared at port.c
 */
void xPortPendSVHandler( void );
void xPortSysTickHandler( void );
void vPortSVCHandler( void );
/** @} */

/**
 * By invoking the RTOS interrupt functions in this wrapper surrounded by run-time statistics hooks, we
 * can measure the time interrupts are using, while subtracting the runtime from the tasks simultaneously
 *
 * Update: 02/2018:
 * Calling ISR in a wrapper causes RTOS crash when a task is using floating-point operations
 * We need to fully investigate it before re-enabling vRunTimeStatIsrEntry() and vRunTimeStatIsrExit()
 */
static void invoke_function_in_wrapper(function__void_f function) {
  // vRunTimeStatIsrEntry();
  function();
  // vRunTimeStatIsrExit();
}

/**
 * The *_wrapper() functions are meant to invoke FreeRTOS interrupts, but with run-time statistics hooks
 */
void vPortSVCHandler_wrapper(void) { invoke_function_in_wrapper(vPortSVCHandler); }
void xPortPendSVHandler_wrapper(void) { invoke_function_in_wrapper(xPortPendSVHandler); }
void xPortSysTickHandler_wrapper(void) { invoke_function_in_wrapper(xPortSysTickHandler); }
