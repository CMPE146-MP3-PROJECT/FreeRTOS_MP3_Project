#pragma once

#include "gpio.h"
#include "uart.h"

#include "FreeRTOS.h"
#include "queue.h"

/**
 * This function can be used to iniitalize UART3
 * You can modify the queue sizes based on your preference
 */
static inline void uart3_init(void) {
  uart__init(UART__3, clock__get_peripheral_clock_hz(), 115200);

  // UART3 is on P4.28, P4.29
  gpio__construct_with_function(GPIO__PORT_4, 28, GPIO__FUNCTION_2);
  gpio__construct_with_function(GPIO__PORT_4, 29, GPIO__FUNCTION_2);

  static StaticQueue_t rxq_struct;
  static StaticQueue_t txq_struct;
  static uint8_t rxq_storage[1024];
  static uint8_t txq_storage[1024];

  QueueHandle_t rxq_handle = xQueueCreateStatic(sizeof(rxq_storage), sizeof(char), rxq_storage, &rxq_struct);
  QueueHandle_t txq_handle = xQueueCreateStatic(sizeof(txq_storage), sizeof(char), txq_storage, &txq_struct);

  uart__enable_queues(UART__3, txq_handle, rxq_handle);
}