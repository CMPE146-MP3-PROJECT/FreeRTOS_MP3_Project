#include "uart_lab.h"
//#include "lpc40xx.h"
//#include "lpc_peripherals.h"

void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {
  // Refer to LPC User manual and setup the register bits correctly
  // The first page of the UART chapter has good instructions
  // a) Power on Peripheral
  // b) Setup DLL, DLM, FDR, LCR registers
  const uint16_t divider_16_bit = peripheral_clock / (16 * baud_rate);
  if (uart == UART_2) {
    LPC_SC->PCONP |= (1 << 24); // UART 2 power enable

    LPC_UART2->LCR |= (1 << 7); // enable divisor latch
    LPC_UART2->DLM = (divider_16_bit >> 8) & 0xFF;
    LPC_UART2->DLL = (divider_16_bit >> 0) & 0xFF; // setup baud rate for UART2
    LPC_UART2->LCR &= ~(1 << 7);                   // disable divisor latch

    LPC_UART2->FDR |= (1 << 4);              // reset MULVAL
    LPC_UART2->LCR |= ((3 << 0) | (0 << 2)); // 8-bit character length
  } else if (uart == UART_3) {
    LPC_SC->PCONP |= (1 << 25); // UART 3 power enable

    LPC_UART3->LCR |= (1 << 7); // enable divisor latch
    LPC_UART3->DLM = (divider_16_bit >> 8) & 0xFF;
    LPC_UART3->DLL = (divider_16_bit >> 0) & 0xFF; // setup baud rate for UART2
    LPC_UART3->LCR &= ~(1 << 7);                   // disable divisor latch

    LPC_UART3->FDR |= (1 << 4);              // reset MULVAL
    LPC_UART3->LCR |= ((3 << 0) | (0 << 2)); // 8-bit character length
  }
}

// Read the byte from RBR and actually save it to the pointer
bool uart_lab__polled_get(uart_number_e uart, char *input_byte) {
  // a) Check LSR for Receive Data Ready
  // b) Copy data from RBR register to input_bytes
  bool poll_status = false;
  if (uart == UART_2) {
    while (!(LPC_UART2->LSR & (1 << 0))) {
      ; // wait if The UARTn receiver FIFO is empty
    }
    *input_byte = LPC_UART2->RBR & 0xff;
    poll_status = true;
  } else if (uart == UART_3) {
    while (!(LPC_UART3->LSR & (1 << 0))) {
      ; // wait if The UARTn receiver FIFO is empty
    }
    *input_byte = LPC_UART3->RBR & 0xff;
    poll_status = true;
  }
  return poll_status;
}

bool uart_lab__polled_put(uart_number_e uart, char output_byte) {
  // a) Check LSR for Transmit Hold Register Empty
  // b) Copy output_byte to THR register
  bool poll_status = false;
  if (uart == UART_2) {
    while (!(LPC_UART2->LSR & (1 << 5))) {
      ; // wait if UnTHR contains valid data.
    }
    LPC_UART2->LCR &= ~(1 << 7); /// disable divisor latch before UARTn Transmit
    LPC_UART2->THR = output_byte;
    poll_status = true;
  } else if (uart == UART_3) {
    while (!(LPC_UART3->LSR & (1 << 5))) {
      ; // wait if UnTHR contains valid data.
    }
    LPC_UART3->LCR &= ~(1 << 7); /// disable divisor latch before UARTn Transmit
    LPC_UART3->THR = output_byte;
    poll_status = true;
  }
  return poll_status;
}
/// part 2 UART interrupt
// TODO: Implement the header file for exposing public functions (non static)

// The idea is that you will use interrupts to input data to FreeRTOS queue
// Then, instead of polling, your tasks can sleep on data that we can read from the queue
// Private queue handle of our uart_lab.c
static QueueHandle_t richie_uart_rx_queue;

// Private function of our uart_lab.c
static void richie_receive_interrupt_u2(void) {
  // TODO: Read the IIR register to figure out why you got interrupted

  // TODO: Based on IIR status, read the LSR register to confirm if there is data to be read
  while (LPC_UART2->IIR & (1 << 0)) {
    ; // wait when not interrupt is pending
  }
  // TODO: Based on LSR status, read the RBR register and input the data to the RX Queue
  while (LPC_UART2->IIR & (1 << 2)) { // when Receive Data Available (RDA = 1).
    if (LPC_UART2->LSR & (1 << 0)) {  // and The UARTn receiver FIFO is not empty
      const char byte = LPC_UART2->RBR & 0xff;
      xQueueSendFromISR(richie_uart_rx_queue, &byte, NULL);
    } else {
      fprintf(stderr, "data not ready");
    }
  }
}

static void richie_receive_interrupt_u3(void) {
  // TODO: Read the IIR register to figure out why you got interrupted

  // TODO: Based on IIR status, read the LSR register to confirm if there is data to be read
  while (LPC_UART3->IIR & (1 << 0)) {
    ; // wait when not interrupt is pending
  }
  // TODO: Based on LSR status, read the RBR register and input the data to the RX Queue
  while (LPC_UART3->IIR & (1 << 2)) { // when Receive Data Available (RDA = 1).
    if (LPC_UART3->LSR & (1 << 0)) {  // and The UARTn receiver FIFO is not empty
      const char byte = LPC_UART3->RBR & 0xff;
      xQueueSendFromISR(richie_uart_rx_queue, &byte, NULL);
    } else {
      fprintf(stderr, "data not ready");
    }
  }
}

// Public function to enable UART interrupt
// TODO Declare this at the header file
void uart__enable_receive_interrupt(uart_number_e uart_number) {
  // TODO: Use lpc_peripherals.h to attach your interrupt
  // NVIC_EnableIRQ(UART2_IRQn);
  // NVIC_EnableIRQ(UART3_IRQn);
  if (uart_number == UART_2) {
    // NVIC_EnableIRQ(UART2_IRQn);
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART2, richie_receive_interrupt_u2, "unused");
    LPC_UART2->IER |= (1 << 0); // set bit 0 to enable RBR interrupt
  } else if (uart_number == UART_3) {
    // NVIC_EnableIRQ(UART3_IRQn);
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART3, richie_receive_interrupt_u3, "unused");
    LPC_UART3->IER |= (1 << 0); // set bit 0 to enable RBR interrupt
  }
  // TODO: Enable UART receive interrupt by reading the LPC User manual
  // Hint: Read about the IER register
  richie_uart_rx_queue = xQueueCreate(3, sizeof(char));
}

// Public function to get a char from the queue (this function should work without modification)
// TODO: Declare this at the header file
bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  return xQueueReceive(richie_uart_rx_queue, input_byte, timeout);
}
