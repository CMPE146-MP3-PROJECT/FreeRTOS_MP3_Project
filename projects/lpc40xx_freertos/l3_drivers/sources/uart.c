#include "uart.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lpc40xx.h"
#include "lpc_peripherals.h"

/// Alias the LPC defined typedef in case we have to define it differently for a different CPU
typedef LPC_UART_TypeDef lpc_uart;

/**
 * Each UART struct is composed of its register pointers, peripheral_id
 * and FreeRTOS queues to support interrupt driven implementation
 */
typedef struct {
  lpc_uart *registers;
  const char *rtos_isr_trace_name;
  QueueHandle_t queue_transmit;
  QueueHandle_t queue_receive;
} uart_s;

/// @{ ISR functions for all UARTs; these are lightweight that simply call uart__isr_common()
static void uart0_isr(void);
static void uart1_isr(void);
static void uart2_isr(void);
static void uart3_isr(void);
/// @}

static void uart__isr_common(uart_s *uart_type); ///< Common function for all UART ISRs

/*******************************************************************************
 *
 *               P R I V A T E    D A T A    D E F I N I T I O N S
 *
 ******************************************************************************/

/**
 * Some UARTs have a different memory map, but it matches the base registers, so
 * we can use the same memory map to provide a generic driver for all UARTs
 *
 * UART1 for instance has same registers like UART0, but has additional modem control registers
 * but these extra registers are at the end of the memory map that matches with UART0
 */
static uart_s uarts[] = {
    {(lpc_uart *)LPC_UART0, "Uart0"},
    {(lpc_uart *)LPC_UART1, "Uart1"},
    {(lpc_uart *)LPC_UART2, "Uart2"},
    {(lpc_uart *)LPC_UART3, "Uart3"},
};

static void (*const uart__isrs[])(void) = {uart0_isr, uart1_isr, uart2_isr, uart3_isr};
static const lpc_peripheral_e uart_peripheral_ids[] = {LPC_PERIPHERAL__UART0, LPC_PERIPHERAL__UART1,
                                                       LPC_PERIPHERAL__UART2, LPC_PERIPHERAL__UART3};

/*******************************************************************************
 *
 *                     P R I V A T E    F U N C T I O N S
 *
 ******************************************************************************/

static bool uart__is_receive_queue_enabled(uart_e uart) { return (NULL != uarts[uart].queue_receive); }
static bool uart__is_transmit_queue_enabled(uart_e uart) { return (NULL != uarts[uart].queue_transmit); }

static void uart__wait_for_transmit_to_complete(lpc_uart *uart_regs) {
  const uint32_t transmitter_empty = (1 << 5);
  while (!(uart_regs->LSR & transmitter_empty)) {
  }
}

static bool uart__load_pending_transmit_bytes(uart_s *uart_type) {
  const size_t hw_fifo_size = 16;

  char transmit_byte = 0;
  bool context_switch_required = false;
  BaseType_t higher_priority_task_woke = 0;

  for (size_t counter = 0; counter < hw_fifo_size; counter++) {
    if (xQueueReceiveFromISR(uart_type->queue_transmit, &transmit_byte, &higher_priority_task_woke)) {
      uart_type->registers->THR = transmit_byte;
      if (higher_priority_task_woke) {
        context_switch_required = true;
      }
    } else {
      break;
    }
  }

  return context_switch_required;
}

static bool uart__clear_receive_fifo(uart_s *uart_type) {
  const uint32_t char_available_bitmask = (1 << 0);

  bool context_switch_required = false;
  BaseType_t higher_priority_task_woke = 0;

  /**
   * While receive Hardware FIFO not empty, keep queuing the data. Even if xQueueSendFromISR()
   * fails (Queue is full), we still need to read RBR register otherwise interrupt will not clear
   */
  while (uart_type->registers->LSR & char_available_bitmask) {
    const char received_byte = uart_type->registers->RBR;
    xQueueSendFromISR(uart_type->queue_receive, &received_byte, &higher_priority_task_woke);

    if (higher_priority_task_woke) {
      context_switch_required = true;
    }
  }

  return context_switch_required;
}

static void uart__enable_receive_and_transmit_interrupts(uart_e uart) {
  uart_s *uart_type = &uarts[uart];
  lpc_peripheral__enable_interrupt(uart_peripheral_ids[uart], uart__isrs[uart], uart_type->rtos_isr_trace_name);

  const uint32_t enable_rx_tx_fifo = (1 << 0) | (1 << 6);
  const uint32_t reset_rx_tx_fifo = (1 << 1) | (1 << 2);

  uart_type->registers->FCR = enable_rx_tx_fifo;
  uart_type->registers->FCR = reset_rx_tx_fifo;

  const uint32_t enable_rx_tx_interrupts = (1 << 0) | (1 << 1) | (1 << 2); // B0:Rx, B1: Tx
  uart_type->registers->IER = enable_rx_tx_interrupts;
}

static void uart0_isr(void) { uart__isr_common(&uarts[UART__0]); }
static void uart1_isr(void) { uart__isr_common(&uarts[UART__1]); }
static void uart2_isr(void) { uart__isr_common(&uarts[UART__2]); }
static void uart3_isr(void) { uart__isr_common(&uarts[UART__3]); }

static void uart__isr_common(uart_s *uart_type) {
  bool context_switch_required = false;

  /// Bit Masks of the IIR register 3:1 bits contain interrupt reason.
  typedef enum {
    transmitter_empty = (1 << 0),
    receive_data_available = (2 << 0),
    receive_data_available_timeout = (6 << 0),
  } interrupt_reason_e;

  const interrupt_reason_e interrupt_reason = (interrupt_reason_e)((uart_type->registers->IIR & 0xE) >> 1);

  switch (interrupt_reason) {
  case transmitter_empty:
    context_switch_required = uart__load_pending_transmit_bytes(uart_type);
    break;

  case receive_data_available: // no-break
  case receive_data_available_timeout:
    context_switch_required = uart__clear_receive_fifo(uart_type);
    break;

  default: {
    volatile const uint32_t unused_to_clear_isr = uart_type->registers->LSR;
    (void)unused_to_clear_isr;
    break;
  }
  }

  portEND_SWITCHING_ISR(context_switch_required);
}

/*******************************************************************************
 *
 *                      P U B L I C    F U N C T I O N S
 *
 ******************************************************************************/

void uart__init(uart_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {
  lpc_peripheral__turn_on_power_to(uart_peripheral_ids[uart]);

  const float roundup_offset = 0.5;
  const uint16_t divider = (uint16_t)((peripheral_clock / (16 * baud_rate)) + roundup_offset);
  const uint8_t dlab_bit = (1 << 7);
  const uint8_t eight_bit_datalen = 3;

  // 2-stop bits helps improve baud rate error; you can remove this if bandwidth is critical to you
  const uint8_t stop_bits_is_2 = (1 << 2);

  lpc_uart *uart_regs = uarts[uart].registers;

  uart_regs->LCR = dlab_bit; // Set DLAB bit to access DLM & DLL
  uart_regs->DLM = (divider >> 8) & 0xFF;
  uart_regs->DLL = (divider >> 0) & 0xFF;

  /* Bootloader uses Uart0 fractional dividers and can wreck havoc in our baud rate code, so re-initialize it
   * Lesson learned: DO NOT RELY ON RESET VALUES
   */
  const uint32_t default_reset_fdr_value = (1 << 4);
  uart_regs->FDR = default_reset_fdr_value;
  uart_regs->LCR = eight_bit_datalen | stop_bits_is_2; // DLAB is reset back to zero also
}

bool uart__is_initialized(uart_e uart) {
  return lpc_peripheral__is_powered_on(uart_peripheral_ids[uart]) && (0 != uarts[uart].registers->LCR);
}

bool uart__is_transmit_queue_initialized(uart_e uart) { return uart__is_transmit_queue_enabled(uart); }

bool uart__enable_queues(uart_e uart, QueueHandle_t queue_receive, QueueHandle_t queue_transmit) {
  bool status = false;
  uart_s *uart_type = &uarts[uart];

  // We can only access UART registers after its power has been enabled
  if (uart__is_initialized(uart)) {
    // Ensure that the queues are not already enabled
    if (!uart__is_receive_queue_enabled(uart) && NULL != queue_receive) {
      uart_type->queue_receive = queue_receive;
      const char name[] = {'U', '0' + (char)uart, 'R', 'X', 'Q', '\0'};
      vTraceSetQueueName(queue_receive, name);
      (void)name; // avoid warning if trace is disabled
    }

    if (!uart__is_transmit_queue_enabled(uart) && NULL != queue_transmit) {
      uart_type->queue_transmit = queue_transmit;
      const char name[] = {'U', '0' + (char)uart, 'T', 'X', 'Q', '\0'};
      vTraceSetQueueName(queue_transmit, name);
      (void)name; // avoid warning if trace is disabled
    }

    // Enable peripheral_id interrupt if all is well
    status = uart__is_receive_queue_enabled(uart) && uart__is_transmit_queue_enabled(uart);
    if (status) {
      uart__enable_receive_and_transmit_interrupts(uart);
    }
  }

  return status;
}

bool uart__polled_get(uart_e uart, char *input_byte) {
  bool status = false;

  const bool rtos_is_running = taskSCHEDULER_RUNNING == xTaskGetSchedulerState();
  const bool queue_is_enabled = uart__is_receive_queue_enabled(uart);

  if (uart__is_initialized(uart)) {
    /* If the RTOS is running and queues are enabled, then we will be unable to access the
     * RBR register directly since the interrupt would occur and read out the RBR.
     *
     * So when the user calls this function with the RTOS and queues enabled,
     * then we opt to block forever using uart__get() to provide 'polled' behavior.
     */
    if (rtos_is_running && queue_is_enabled) {
      status = uart__get(uart, input_byte, UINT32_MAX);
    } else {
      lpc_uart *uart_regs = uarts[uart].registers;
      const uint32_t char_available_bitmask = (1 << 0);

      while (!(uart_regs->LSR & char_available_bitmask)) {
      }
      *input_byte = uart_regs->RBR;
    }
  }

  return status;
}

bool uart__polled_put(uart_e uart, char output_byte) {
  bool status = false;
  lpc_uart *uart_regs = uarts[uart].registers;

  if (uart__is_initialized(uart)) {
    status = true;

    // Wait for any prior transmission to complete
    uart__wait_for_transmit_to_complete(uart_regs);
    uart_regs->THR = output_byte;
    uart__wait_for_transmit_to_complete(uart_regs);
  }

  return status;
}

bool uart__get(uart_e uart, char *input_byte, uint32_t timeout_ms) {
  bool status = false;
  const bool rtos_is_running = taskSCHEDULER_RUNNING == xTaskGetSchedulerState();

  /* If a user calls this function without the RTOS running, we fail gracefully.
   * We do not desire to perform polling because that would involve time keeping
   * without an RTOS which increases the driver complexity.
   */
  if (uart__is_receive_queue_enabled(uart) && rtos_is_running) {
    status = xQueueReceive(uarts[uart].queue_receive, input_byte, RTOS_MS_TO_TICKS(timeout_ms));
  }

  return status;
}

bool uart__put(uart_e uart, char output_byte, uint32_t timeout_ms) {
  bool status = false;
  const bool rtos_is_running = taskSCHEDULER_RUNNING == xTaskGetSchedulerState();

  if (uart__is_transmit_queue_enabled(uart) && rtos_is_running) {
    // Deposit to the transmit queue for now
    status = xQueueSend(uarts[uart].queue_transmit, &output_byte, RTOS_MS_TO_TICKS(timeout_ms));

    /* 'Transmit Complete Interrupt' may have already fired when we get here, so if there is no further pending data
     * to be sent, it will not fire again to send any data. Hence, we check here in a critical section if transmit
     * holder register is empty, and kick-off the tranmisssion
     */
    portENTER_CRITICAL();
    {
      lpc_uart *uart_regs = uarts[uart].registers;
      const uint32_t uart_tx_is_idle = (1 << 6);

      if (uart_regs->LSR & uart_tx_is_idle) {
        /* Receive oldest char from the queue to send
         * Since we are inside a critical section, we use FromISR() FreeRTOS API  variant
         */
        if (xQueueReceiveFromISR(uarts[uart].queue_transmit, &output_byte, NULL)) {
          uart_regs->THR = output_byte;
        }
      }
    }
    portEXIT_CRITICAL();
  }

  return status;
}
