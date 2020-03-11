#include <string.h> // memset

#include "FreeRTOS.h"
#include "queue.h"

#include "can_bus.h"

#include "clock.h"           // Get periheral clock
#include "gpio.h"            // Configure pins
#include "lpc_peripherals.h" // Enable interrupt
#include "sys_time.h"        // Timeout if RTOS is not running

/**
 * If non-zero, test code is enabled, and each message sent is self-recepted.
 * You need to either connect a CAN transceiver, or connect RD/TD wires of
 * the board with a 1K resistor for the tests to work.
 *
 * Note that FullCAN and CAN filter is not tested together, but they both work individually.
 */
#define CAN_TESTING 0

#define CAN_STRUCT_PTR(can) (&(can__struct_instances[can]))
#define CAN_ENUM_IS_VALID(x) (can1 == x || can2 == x)

// Used by CAN_CT_ASSERT().  Obtained from http://www.pixelbeat.org/programming/gcc/static_assert.html
#define CAN_ASSERT_CONCAT_(a, b) a##b
#define CAN_ASSERT_CONCAT(a, b) CAN_ASSERT_CONCAT_(a, b)
#define CAN_CT_ASSERT(e) enum { CAN_ASSERT_CONCAT(assert_line_, __LINE__) = 1 / (!!(e)) }

// Make some compile-time (CT) checks :
// Check the sizes of the structs because the size needs to match the HW registers
CAN_CT_ASSERT(2 == sizeof(can_std_id_t));
CAN_CT_ASSERT(4 == sizeof(can_ext_id_t));
CAN_CT_ASSERT(8 == sizeof(can__data_t));
CAN_CT_ASSERT(16 == sizeof(can__msg_t));
CAN_CT_ASSERT(12 == sizeof(can__fullcan_msg_t));

/// Interrupt masks of the CANxIER and CANxICR registers
typedef enum {
  intr_rx = (1 << 0),                             ///< Receive
  intr_tx1 = (1 << 1),                            ///< Transmit 1
  intr_warn = (1 << 2),                           ///< Warning (if error BUS status changes)
  intr_ovrn = (1 << 3),                           ///< Data overrun
  intr_wkup = (1 << 4),                           ///< Wake-up
  intr_epi = (1 << 5),                            ///< Change from error active to error passive or vice versa
  intr_ali = (1 << 6),                            ///< Arbitration lost
  intr_berr = (1 << 7),                           ///< Bus error (happens during each error/retry of a message)
  intr_idi = (1 << 8),                            ///< ID ready (a message was transmitted or aborted)
  intr_tx2 = (1 << 9),                            ///< Transmit 2
  intr_tx3 = (1 << 10),                           ///< Transmit 3
  intr_all_tx = (intr_tx1 | intr_tx2 | intr_tx3), ///< Mask of the 3 transmit buffers
} can_intr_t;

/// Bit mask of SR register indicating which hardware buffer is available
enum {
  tx1_avail = (1 << 2),  ///< Transmit buffer 1 is available
  tx2_avail = (1 << 10), ///< Transmit buffer 2 is available
  tx3_avail = (1 << 18), ///< Transmit buffer 3 is available
  tx_all_avail = (tx1_avail | tx2_avail | tx3_avail),
};

/**
 * Data values of the AFMR register
 * @note Since AFMR is common to both controllers, when bypass mode is enabled,
 *       then ALL messages from ALL CAN controllers will be accepted
 *
 *  Bit1: Bypass    Bit0: ACC Off
 *      0               1               No messages accepted
 *      1               X               All messages accepted
 *      0               0               HW Filter or FullCAN
 */
enum {
  afmr_enabled = 0x00,  ///< Hardware acceptance filtering
  afmr_disabled = 0x01, ///< No messages will be accepted
  afmr_bypass = 0x02,   ///< Bypass mode, all messages will be accepted.  Both 0x02 or 0x03 will work.
  afmr_fullcan = 0x04,  ///< Hardware will receive and store messages per FullCAN mode.
};

/// CAN MOD register values
enum {
  can_mod_normal = 0x00,                            ///< CAN MOD register value to enable the BUS
  can_mod_reset = 0x01,                             ///< CAN MOD register value to reset the BUS
  can_mod_normal_tpm = (can_mod_normal | (1 << 3)), ///< CAN bus enabled with TPM mode bits set
  can_mod_selftest = (1 << 2) | can_mod_normal,     ///< Used to enable global self-test
};

/// Typedef of CAN queues and data
typedef struct {
  LPC_CAN_TypeDef *can_reg_ptr; ///< The pointer to the CAN registers
  QueueHandle_t rx_q;           ///< TX queue
  QueueHandle_t tx_q;           ///< RX queue

  uint16_t dropped_rx_msgs; ///< # of msgs dropped if no space found during the CAN interrupt that queues the RX msgs
  uint16_t rx_q_watermark;  ///< Watermark of the FreeRTOS Rx Queue
  uint16_t tx_q_watermark;  ///< Watermark of the FreeRTOS Tx Queue
  uint16_t tx_msg_count;    ///< Number of messages sent
  uint16_t rx_msg_count;    ///< Number of received messages

  can_void_func_t bus_error;    ///< When serious BUS error occurs
  can_void_func_t data_overrun; ///< When we read the CAN buffer too late for incoming message
} can_struct_t;

/// Structure of both CANs
static can_struct_t can__struct_instances[can_max] = {{LPC_CAN1}, {LPC_CAN2}};

/**
 * This type of CAN interrupt should lead to "bus error", but note that intr_berr is not the right
 * one as that one is triggered upon each type of CAN error which may be a simple "retry" that
 * can be recovered.  intr_epi or intr_warn should work for this selection.
 */
static const can_intr_t can__bus_error_interrupt = intr_epi;

/**
 * Sends a message using an available buffer.  Initially this chose one out of the three buffers but that's
 * a little tricky to use when messages are always queued since one of the 3 buffers can be starved and not
 * get sent.  So therefore some of that logic is #ifdef'd out to only use one HW buffer.
 *
 * @returns true if the message was written to the HW buffer to be sent, otherwise false if the HW buffer(s) are busy.
 *
 * Notes:
 *  - Using the TX message counter and the TPM bit, we can ensure that the HW chooses between the TX1/TX2/TX3
 *    in a round-robin fashion otherwise there is a possibility that if the CAN Tx queue is always full,
 *    a low message ID can be starved even if it was amongst the first ones written using this method call.
 *
 * @warning This should be called from critical section since this method is not thread-safe
 */
static bool can__tx_now(can_struct_t *struct_ptr, can__msg_t *msg_ptr) {
  // 32-bit command of CMR register to start transmission of one of the buffers
  enum {
    go_cmd_invalid = 0,
    go_cmd_tx1 = 0x21,
    go_cmd_tx2 = 0x41,
    go_cmd_tx3 = 0x81,
  };

  LPC_CAN_TypeDef *pCAN = struct_ptr->can_reg_ptr;
  const uint32_t can_sr_reg = pCAN->SR;
  volatile can__msg_t *hw_msg_reg_ptr = NULL;
  uint32_t go_cmd = go_cmd_invalid;

  if (can_sr_reg & tx1_avail) {
    hw_msg_reg_ptr = (can__msg_t *)&(pCAN->TFI1);
    go_cmd = go_cmd_tx1;
  }
  // This logic is disabled as it needs further testing
#if 0
    else if (can_sr_reg & tx2_avail){
        hw_msg_reg_ptr = (can__msg_t*)&(pCAN->TFI2);
        go_cmd = go_cmd_tx2;
    }
    else if (can_sr_reg & tx3_avail){
        hw_msg_reg_ptr = (can__msg_t*)&(pCAN->TFI3);
        go_cmd = go_cmd_tx3;
    }
#endif
  else {
    /* No buffer available, return failure */
    return false;
  }

  /* Copy the CAN message to the HW CAN registers and write the 8 TPM bits.
   * We set TPM bits each time by using the tx_msg_count because otherwise if TX1, and TX2 are always
   * being written with a lower message ID, then TX3 will starve and never be sent.
   */
#if 0
    // Higher number will be sent later, but how do we handle the rollover from 255 to 0 because then the
    // newly written 0 will be sent, and buffer that contains higher TPM can starve.
    const uint8_t tpm = struct_ptr->tx_msg_count;
    msg_ptr->frame |= tpm;
#endif
  *hw_msg_reg_ptr = *msg_ptr;
  struct_ptr->tx_msg_count++;

#if CAN_TESTING
  go_cmd &= (0xF0);
  go_cmd = (1 << 4); /* Self reception */
#endif

  /* Send the message! */
  pCAN->CMR = go_cmd;
  return true;
}

static void can__handle_isr(const can__num_e can) {
  can_struct_t *can_instance_ptr = CAN_STRUCT_PTR(can);
  LPC_CAN_TypeDef *pCAN = can_instance_ptr->can_reg_ptr;

  const uint32_t rbs = (1 << 0);
  const uint32_t ibits = pCAN->ICR;

  UBaseType_t count = 0;
  BaseType_t higher_priority_task_woke = 0;
  can__msg_t msg;

  /* Handle the received message */
  if ((ibits & intr_rx) | (pCAN->GSR & rbs)) {
    if ((count = uxQueueMessagesWaitingFromISR(can_instance_ptr->rx_q)) > can_instance_ptr->rx_q_watermark) {
      can_instance_ptr->rx_q_watermark = count;
    }

    can__msg_t *hw_msg_reg_ptr = (can__msg_t *)&(pCAN->RFS);
    if (xQueueSendFromISR(can_instance_ptr->rx_q, hw_msg_reg_ptr, &higher_priority_task_woke)) {
      can_instance_ptr->rx_msg_count++;
    } else {
      can_instance_ptr->dropped_rx_msgs++;
    }
    pCAN->CMR = 0x04; // Release the receive buffer, no need to bitmask
  }

  /* A transmit finished, send any queued message(s) */
  if (ibits & intr_all_tx) {
    if ((count = uxQueueMessagesWaitingFromISR(can_instance_ptr->tx_q)) > can_instance_ptr->tx_q_watermark) {
      can_instance_ptr->tx_q_watermark = count;
    }
    if (xQueueReceiveFromISR(can_instance_ptr->tx_q, &msg, NULL)) {
      can__tx_now(can_instance_ptr, &msg);
    }
  }

  /* We only enable interrupt when a valid callback exists, so no need
   * to check for the callback function being NULL
   */
  if (ibits & can__bus_error_interrupt) {
    can_instance_ptr->bus_error(ibits);
  }
  if (ibits & intr_ovrn) {
    can_instance_ptr->data_overrun(ibits);
  }

  portEND_SWITCHING_ISR(higher_priority_task_woke);
}

/**
 * This interrupt is shared between CAN1, and CAN2
 * We service both CAN peripherals if they have both interrupted
 */
static void can__isr(void) {
  /* Reading registers without CAN powered up will cause DABORT exception */
  if (lpc_peripheral__is_powered_on(LPC_PERIPHERAL__CAN0)) {
    can__handle_isr(can1);
  }

  if (lpc_peripheral__is_powered_on(LPC_PERIPHERAL__CAN1)) {
    can__handle_isr(can2);
  }
}

//
// Public functions
//
bool can__init(can__num_e can, uint32_t baudrate_kbps, uint16_t rxq_size, uint16_t txq_size, can_void_func_t bus_off_cb,
               can_void_func_t data_ovr_cb) {
  if (!CAN_ENUM_IS_VALID(can)) {
    return false;
  }

  can_struct_t *can_instance_ptr = CAN_STRUCT_PTR(can);
  LPC_CAN_TypeDef *pCAN = can_instance_ptr->can_reg_ptr;
  bool failed = true;

  /* Enable CAN Power, and select the PINS
   * CAN1 is at P0.0, P0.1 and P0.21, P0.22
   * CAN2 is at P0.4, P0.5 and P2.7,  P2.8
   * On SJ-One board, we have P0.0, P0.1, and P2.7, P2.8
   */
  if (can1 == can) {
    lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__CAN0);
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__CAN0, can__isr, "CAN0");

    gpio__set_function((gpio_s){.port_number = GPIO__PORT_0, .pin_number = 0}, GPIO__FUNCTION_1);
    gpio__set_function((gpio_s){.port_number = GPIO__PORT_0, .pin_number = 1}, GPIO__FUNCTION_1);
  } else if (can2 == can) {
    lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__CAN1);
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__CAN1, can__isr, "CAN1");

    gpio__set_function((gpio_s){.port_number = GPIO__PORT_2, .pin_number = 7}, GPIO__FUNCTION_1);
    gpio__set_function((gpio_s){.port_number = GPIO__PORT_2, .pin_number = 8}, GPIO__FUNCTION_1);
  } else {
    // Should never happen
  }

  /* Create the queues with minimum size of 1 to avoid NULL pointer reference */
  if (!can_instance_ptr->rx_q) {
    can_instance_ptr->rx_q = xQueueCreate(rxq_size ? rxq_size : 1, sizeof(can__msg_t));
  }
  if (!can_instance_ptr->tx_q) {
    can_instance_ptr->tx_q = xQueueCreate(txq_size ? txq_size : 1, sizeof(can__msg_t));
  }

  pCAN->MOD = can_mod_reset;
  pCAN->IER = 0x0; // Disable All CAN Interrupts
  pCAN->GSR = 0x0; // Clear error counters
  pCAN->CMR = 0xE; // Abort Tx, release Rx, clear data over-run

  /**
   * About the AFMR register :
   *                      B0              B1
   * Filter Mode |    AccOff bit  |   AccBP bit   |   CAN Rx interrupt
   * Off Mode             1               0           No messages accepted
   * Bypass Mode          X               1           All messages accepted
   * FullCAN              0               0           HW acceptance filtering
   */
  LPC_CANAF->AFMR = afmr_disabled;

  // Clear pending interrupts and the CAN Filter RAM
  LPC_CANAF_RAM->mask[0] = pCAN->ICR;
  memset((void *)&(LPC_CANAF_RAM->mask[0]), 0, sizeof(LPC_CANAF_RAM->mask));

  /* Zero out the filtering registers */
  LPC_CANAF->SFF_sa = 0;
  LPC_CANAF->SFF_GRP_sa = 0;
  LPC_CANAF->EFF_sa = 0;
  LPC_CANAF->EFF_GRP_sa = 0;
  LPC_CANAF->ENDofTable = 0;

  /* Do not accept any messages until CAN filtering is enabled */
  LPC_CANAF->AFMR = afmr_disabled;

  /* Set the baud-rate. You can verify the settings by visiting:
   * http://www.kvaser.com/en/support/bit-timing-calculator.html
   */
  do {
    const uint32_t baud_divider = clock__get_peripheral_clock_hz() / (1000 * baudrate_kbps);
    const uint32_t SJW = 3;
    const uint32_t SAM = 0;
    uint32_t BRP = 0, TSEG1 = 0, TSEG2 = 0, NT = 0;

    /* Calculate suitable nominal time value
     * NT (nominal time) = (TSEG1 + TSEG2 + 3)
     * NT <= 24
     * TSEG1 >= 2*TSEG2
     */
    failed = true;
    for (NT = 24; NT > 0; NT -= 2) {
      if ((baud_divider % NT) == 0) {
        BRP = baud_divider / NT - 1;
        NT--;
        TSEG2 = (NT / 3) - 1;
        TSEG1 = NT - (NT / 3) - 1;
        failed = false;
        break;
      }
    }

    if (!failed) {
      pCAN->BTR = (SAM << 23) | (TSEG2 << 20) | (TSEG1 << 16) | (SJW << 14) | BRP;
      // CANx->BTR = 0x002B001D; // 48Mhz 100Khz
    }
  } while (0);

  /* If everything okay so far, enable the CAN interrupts */
  if (!failed) {
    /* At minimum, we need Rx/Tx interrupts */
    pCAN->IER = (intr_rx | intr_all_tx);

    /* Enable BUS-off interrupt and callback if given */
    if (bus_off_cb) {
      can_instance_ptr->bus_error = bus_off_cb;
      pCAN->IER |= can__bus_error_interrupt;
    }
    /* Enable data-overrun interrupt and callback if given */
    if (data_ovr_cb) {
      can_instance_ptr->data_overrun = data_ovr_cb;
      pCAN->IER |= intr_ovrn;
    }
  }

  /* return true if all is well */
  return (false == failed);
}

bool can__tx(can__num_e can, can__msg_t *msg_ptr, uint32_t timeout_ms) {
  if (!CAN_ENUM_IS_VALID(can) || !msg_ptr || can__is_bus_off(can)) {
    return false;
  }

  bool status = false;
  can_struct_t *can_instance_ptr = CAN_STRUCT_PTR(can);
  LPC_CAN_TypeDef *CANx = can_instance_ptr->can_reg_ptr;

  // Enqueue the message first to preserve FIFO order of transmission through can__tx()
  if (taskSCHEDULER_RUNNING == xTaskGetSchedulerState()) {
    status = xQueueSend(can_instance_ptr->tx_q, msg_ptr, RTOS_MS_TO_TICKS(timeout_ms));
  } else {
    status = xQueueSend(can_instance_ptr->tx_q, msg_ptr, 0);
  }

  /* There is possibility that before we queued the message, we got interrupted and all hw buffers were emptied
   * meanwhile, and our queued message will now sit in the queue forever until another Tx interrupt takes place.
   */
  taskENTER_CRITICAL();
  do {
    can__msg_t msg;
    if (tx_all_avail == (CANx->SR & tx_all_avail) && xQueueReceive(can_instance_ptr->tx_q, &msg, 0)) {
      status = can__tx_now(can_instance_ptr, &msg);
    }
  } while (0);
  taskEXIT_CRITICAL();

  return status;
}

bool can__rx(can__num_e can, can__msg_t *msg_ptr, uint32_t timeout_ms) {
  bool status = false;

  if (CAN_ENUM_IS_VALID(can) && (NULL != msg_ptr)) {
    if (taskSCHEDULER_RUNNING == xTaskGetSchedulerState()) {
      status = xQueueReceive(CAN_STRUCT_PTR(can)->rx_q, msg_ptr, RTOS_MS_TO_TICKS(timeout_ms));
    } else {
      const uint64_t msg_timeout = sys_time__get_uptime_ms() + timeout_ms;
      while (!(status = xQueueReceive(CAN_STRUCT_PTR(can)->rx_q, msg_ptr, 0))) {
        if (sys_time__get_uptime_ms() > msg_timeout) {
          break;
        }
      }
    }
  }

  return status;
}

bool can__is_bus_off(can__num_e can) {
  const uint32_t bus_off_mask = (1 << 7);
  return (!CAN_ENUM_IS_VALID(can)) ? true : !!(CAN_STRUCT_PTR(can)->can_reg_ptr->GSR & bus_off_mask);
}

void can__reset_bus(can__num_e can) {
  if (CAN_ENUM_IS_VALID(can)) {
    CAN_STRUCT_PTR(can)->can_reg_ptr->MOD = can_mod_reset;

#if CAN_TESTING
    CAN_STRUCT_PTR(can)->can_reg_ptr->MOD = can_mod_selftest;
#else
    CAN_STRUCT_PTR(can)->can_reg_ptr->MOD = can_mod_normal_tpm;
#endif
  }
}

uint16_t can__get_rx_watermark(can__num_e can) {
  return CAN_ENUM_IS_VALID(can) ? CAN_STRUCT_PTR(can)->rx_q_watermark : 0;
}

uint16_t can__get_tx_watermark(can__num_e can) {
  return CAN_ENUM_IS_VALID(can) ? CAN_STRUCT_PTR(can)->tx_q_watermark : 0;
}

uint16_t can__get_tx_count(can__num_e can) { return CAN_ENUM_IS_VALID(can) ? CAN_STRUCT_PTR(can)->tx_msg_count : 0; }

uint16_t can__get_rx_count(can__num_e can) { return CAN_ENUM_IS_VALID(can) ? CAN_STRUCT_PTR(can)->rx_msg_count : 0; }

uint16_t can__get_rx_dropped_count(can__num_e can) {
  return CAN_ENUM_IS_VALID(can) ? CAN_STRUCT_PTR(can)->dropped_rx_msgs : 0;
}

void can__bypass_filter_accept_all_msgs(void) { LPC_CANAF->AFMR = afmr_bypass; }

can_std_id_t can__generate_standard_id(can__num_e can, uint16_t id) {
  /* SCC in datasheet is defined as can controller - 1 */
  const uint16_t scc = (can);
  can_std_id_t ret;

  ret.can_num = scc;
  ret.disable = (0xffff == id) ? 1 : 0;
  ret.fc_intr = 0;
  ret.id = id;

  return ret;
}

can_ext_id_t can__generate_extended_id(can__num_e can, uint32_t id) {
  /* SCC in datasheet is defined as can controller - 1 */
  const uint16_t scc = (can);
  can_ext_id_t ret;

  ret.can_num = scc;
  ret.id = id;

  return ret;
}

bool can__fullcan_add_entry(can__num_e can, can_std_id_t id1, can_std_id_t id2) {
  /* Return if invalid CAN given */
  if (!CAN_ENUM_IS_VALID(can)) {
    return false;
  }

  /* Check for enough room for more FullCAN entries
   * Each entry takes 2-byte entry, and 12-byte message RAM.
   */
  const uint16_t existing_entries = can__fullcan_get_num_entries();
  const uint16_t size_per_entry = sizeof(can_std_id_t) + sizeof(can__fullcan_msg_t);
  if ((existing_entries * size_per_entry) >= sizeof(LPC_CANAF_RAM->mask)) {
    return false;
  }

  /* Locate where we should write the next entry */
  uint8_t *base = (uint8_t *)&(LPC_CANAF_RAM->mask[0]);
  uint8_t *next_entry_ptr = base + LPC_CANAF->SFF_sa;

  /* Copy the new entry into the RAM filter */
  LPC_CANAF->AFMR = afmr_disabled;
  do {
    const uint32_t entries = ((uint32_t)id2.raw & UINT16_MAX) | ((uint32_t)id1.raw << 16);
    *(uint32_t *)(next_entry_ptr) = entries;

    /* The new start of Standard Frame Filter is after the two new entries */
    const uint32_t new_sff_sa = LPC_CANAF->SFF_sa + sizeof(id1) + sizeof(id2);
    LPC_CANAF->SFF_sa = new_sff_sa;

    /* Next filters start at SFF_sa (they are disabled) */
    LPC_CANAF->SFF_GRP_sa = new_sff_sa;
    LPC_CANAF->EFF_sa = new_sff_sa;
    LPC_CANAF->EFF_GRP_sa = new_sff_sa;
    LPC_CANAF->ENDofTable = new_sff_sa;
  } while (0);
  LPC_CANAF->AFMR = afmr_fullcan;

  return true;
}

can__fullcan_msg_t *can__fullcan_get_entry_ptr(can_std_id_t fc_id) {
  /* Number of entries depends on how far SFF_sa is from base of 0 */
  const uint16_t num_entries = can__fullcan_get_num_entries();
  uint16_t idx = 0;

  /* The FullCAN entries are at the base of the CAN RAM */
  const can_std_id_t *id_list = (can_std_id_t *)&(LPC_CANAF_RAM->mask[0]);

  /* Find the standard ID entered into the RAM
   * Once we find the ID, its message's RAM location is after
   * LPC_CANAF->ENDofTable based on the index location.
   *
   * Note that due to MSB/LSB of the CAN RAM, we check in terms of 16-bit WORDS
   * and LSB word match means we will find it at index + 1, and MSB word match
   * means we will find it at the index.
   */
  for (idx = 0; idx < num_entries; idx += 2) {
    if (id_list[idx].id == fc_id.id) {
      ++idx;
      break;
    }
    if (id_list[idx + 1].id == fc_id.id) {
      break;
    }
  }

  can__fullcan_msg_t *real_entry = NULL;
  if (idx < num_entries) {
    /* If we find an index, we have to convert it to the actual message pointer */
    can__fullcan_msg_t *base_msg_entry =
        (can__fullcan_msg_t *)(((uint8_t *)&(LPC_CANAF_RAM->mask[0])) + LPC_CANAF->ENDofTable);
    real_entry = (base_msg_entry + idx);
  }

  return real_entry;
}

bool can__fullcan_read_msg_copy(can__fullcan_msg_t *pMsg, can__fullcan_msg_t *pMsgCopy) {
  const uint8_t *can_ram_base = (uint8_t *)&(LPC_CANAF_RAM->mask[0]);
  const uint8_t *start = can_ram_base + LPC_CANAF->ENDofTable;     // Actual FullCAN msgs are stored after this
  const uint8_t *end = can_ram_base + sizeof(LPC_CANAF_RAM->mask); // Last byte of CAN RAM + 1
  bool new_msg_received = false;

  /* Validate the input pointers.  pMsg must be within range of our RAM filter
   * where the actual FullCAN message should be stored at
   */
  const uint8_t *ptr = (uint8_t *)pMsg;
  if (ptr < start || ptr >= end || !pMsgCopy) {
    return false;
  }

  /* If semaphore bits change, then HW has updated the message so read it again.
   * After HW writes new message, semaphore bits are changed to 0b11.
   */
  while (0 != pMsg->semphr) {
    new_msg_received = true;
    pMsg->semphr = 0;
    *pMsgCopy = *pMsg;
  }

  return new_msg_received;
}

uint8_t can__fullcan_get_num_entries(void) { return LPC_CANAF->SFF_sa / sizeof(can_std_id_t); }

bool can__setup_filter(const can_std_id_t *std_id_list, uint16_t sid_cnt, const can_std_grp_id_t *std_group_id_list,
                       uint16_t sgp_cnt, const can_ext_id_t *ext_id_list, uint16_t eid_cnt,
                       const can_ext_grp_id_t *ext_group_id_list, uint16_t egp_cnt) {
  bool ok = true;
  uint32_t i = 0;
  uint32_t temp32 = 0;

  // Count of standard IDs must be even
  if (sid_cnt & 1) {
    return false;
  }

  LPC_CANAF->AFMR = afmr_disabled;
  do {
    /* Filter RAM is after the FulLCAN entries */
    uint32_t can_ram_base_addr = (uint32_t) & (LPC_CANAF_RAM->mask[0]);

    /* FullCAN entries take up 2 bytes each at beginning RAM, and 12-byte sections at the end */
    const uint32_t can_ram_end_addr =
        can_ram_base_addr + sizeof(LPC_CANAF_RAM->mask) - (sizeof(can__fullcan_msg_t) * can__fullcan_get_num_entries());

    /* Our filter RAM is after FullCAN entries */
    uint32_t *ptr = (uint32_t *)(can_ram_base_addr + LPC_CANAF->SFF_sa);

/* macro to swap top and bottom 16-bits of 32-bit DWORD */
#define CAN_swap32(t32)                                                                                                \
  do {                                                                                                                 \
    t32 = (t32 >> 16) | (t32 << 16);                                                                                   \
  } while (0)

/**
 * Standard ID list and group list need to swapped otherwise setting the wrong
 * filter will make the CAN ISR go into a loop for no apparent reason.
 * It looks like the filter data is motorolla big-endian format.
 * See "configuration example 5" in CAN chapter.
 */
#define can__add_filter_list(list, ptr, end, cnt, entry_size, swap)                                                    \
  do {                                                                                                                 \
    if (NULL != list) {                                                                                                \
      if ((uint32_t)ptr + (cnt * entry_size) < end) {                                                                  \
        for (i = 0; i < (cnt * entry_size) / 4; i++) {                                                                 \
          if (swap) {                                                                                                  \
            temp32 = ((uint32_t *)list)[i];                                                                            \
            CAN_swap32(temp32);                                                                                        \
            ptr[i] = temp32;                                                                                           \
          } else {                                                                                                     \
            ptr[i] = ((uint32_t *)list)[i];                                                                            \
          }                                                                                                            \
        }                                                                                                              \
        ptr += (cnt * entry_size) / 4;                                                                                 \
      } else {                                                                                                         \
        ok = false;                                                                                                    \
      }                                                                                                                \
    }                                                                                                                  \
  } while (0)

    /* The sa (start addresses) are byte address offset from CAN RAM
     * and must be 16-bit (WORD) aligned
     * LPC_CANAF->SFF_sa should already be setup by FullCAN if used, or
     * set to zero by the can init function.
     */
    can__add_filter_list(std_id_list, ptr, can_ram_end_addr, sid_cnt, sizeof(can_std_id_t), true);

    LPC_CANAF->SFF_GRP_sa = ((uint32_t)ptr - can_ram_base_addr);
    can__add_filter_list(std_group_id_list, ptr, can_ram_end_addr, sgp_cnt, sizeof(can_std_grp_id_t), true);

    LPC_CANAF->EFF_sa = ((uint32_t)ptr - can_ram_base_addr);
    can__add_filter_list(ext_id_list, ptr, can_ram_end_addr, eid_cnt, sizeof(can_ext_id_t), false);

    LPC_CANAF->EFF_GRP_sa = ((uint32_t)ptr - can_ram_base_addr);
    can__add_filter_list(ext_group_id_list, ptr, can_ram_end_addr, egp_cnt, sizeof(can_ext_grp_id_t), false);

    /* End of table is where the FullCAN messages are stored */
    LPC_CANAF->ENDofTable = ((uint32_t)ptr - can_ram_base_addr);
  } while (0);

  /* If there was no FullCAN entry, then SFF_sa will be zero.
   * If it was zero, we just enable the AFMR, but if it was not zero, that means
   * FullCAN entry was added, so we restore AMFR to fullcan enable
   */
  LPC_CANAF->AFMR = (0 == LPC_CANAF->SFF_sa) ? afmr_enabled : afmr_fullcan;

  return ok;
}

#if CAN_TESTING
#include <stdio.h>
#define CAN_ASSERT(x)                                                                                                  \
  if (!(x)) {                                                                                                          \
    printf("Failed at %i, BUS: %s MOD: 0x%08x, GSR: 0x%08x\n"                                                          \
           "IER/ICR: 0x%08X/0x%08x BTR: 0x%08x"                                                                        \
           "\nLine %i: %s\n",                                                                                          \
           __LINE__, can__is_bus_off(can1) ? "OFF" : "ON", (int)LPC_CAN1->MOD, (int)LPC_CAN1->GSR, (int)LPC_CAN1->IER, \
           (int)LPC_CAN1->ICR, (int)LPC_CAN1->BTR, __LINE__, #x);                                                      \
    return false;                                                                                                      \
  }
void can__test_b bool can__test(void) {
  uint32_t i = 0;

#define can_test_msg(msg, id, rxtrue)                                                                                  \
  do {                                                                                                                 \
    printf("Send ID: 0x%08X\n", id);                                                                                   \
    msg.msg_id = id;                                                                                                   \
    CAN_ASSERT(can__tx(can1, &msg, 0));                                                                                \
    msg.msg_id = 0;                                                                                                    \
    CAN_ASSERT(rxtrue == can__rx(can1, &msg, 10));                                                                     \
    if (rxtrue)                                                                                                        \
      CAN_ASSERT(id == msg.msg_id);                                                                                    \
  } while (0)

  printf("  Test init()\n");
  CAN_ASSERT(!can__init(can_max, 100, 0, 0, NULL, NULL));
  CAN_ASSERT(can__init(can1, 100, 5, 5, can__test_bufoff_cb, can__test_bufovr_cb));
  CAN_ASSERT(LPC_CAN1->MOD == can_mod_reset);
  can__bypass_filter_accept_all_msgs();

  CAN_ASSERT(can__struct_instances[0].rx_q != NULL);
  CAN_ASSERT(can__struct_instances[0].tx_q != NULL);
  CAN_ASSERT(LPC_CANAF->SFF_sa == 0);
  CAN_ASSERT(LPC_CANAF->SFF_GRP_sa == 0);
  CAN_ASSERT(LPC_CANAF->EFF_sa == 0);
  CAN_ASSERT(LPC_CANAF->EFF_GRP_sa == 0);
  CAN_ASSERT(LPC_CANAF->ENDofTable == 0);

  can__reset_bus(can1);
  CAN_ASSERT(LPC_CAN1->MOD == can_mod_selftest);

  /* Create a message, and test tx with bad input */
  uint32_t id = 0x100;
  can__msg_t msg;
  memset(&msg, 0, sizeof(msg));
  msg.frame = 0;
  msg.msg_id = id;
  msg.frame_fields.is_29bit = 0;
  msg.frame_fields.data_len = 8;
  msg.data.qword = 0x1122334455667788;
  CAN_ASSERT(!can__tx(can_max, &msg, 0)); // Invalid CAN
  CAN_ASSERT(!can__rx(can1, NULL, 0));    // Invalid message pointer

  /* Send msg and test receive */
  printf("  Test Tx/Rx\n");
  can_test_msg(msg, 0x100, true);
  can_test_msg(msg, 0x200, true);
  can_test_msg(msg, 0x300, true);
  can_test_msg(msg, 0x400, true);
  can_test_msg(msg, 0x500, true);

  const can_std_id_t slist[] = {
      can__generate_standard_id(can1, 0x100), can__generate_standard_id(can1, 0x110), // 2 entries
      can__generate_standard_id(can1, 0x120), can__generate_standard_id(can1, 0x130)  // 2 entries
  };
  const can_std_grp_id_t sglist[] = {
      {can__generate_standard_id(can1, 0x200), can__generate_standard_id(can1, 0x210)}, // Group 1
      {can__generate_standard_id(can1, 0x220), can__generate_standard_id(can1, 0x230)}  // Group 2
  };
  const can_ext_id_t elist[] = {can__generate_extended_id(can1, 0x7500), can__generate_extended_id(can1, 0x8500)};
  const can_ext_grp_id_t eglist[] = {
      {can__generate_extended_id(can1, 0xA000), can__generate_extended_id(can1, 0xB000)}}; // Group 1

  /* Test filter setup */
  printf("  Test filter setup\n");
  can__setup_filter(slist, 4, sglist, 2, elist, 2, eglist, 1);

  /* We use offset of zero if 2 FullCAN messages are added, otherwise 4 if none were added above */
  const uint8_t offset = 4;
  CAN_ASSERT(LPC_CANAF->SFF_sa == 4 - offset);
  CAN_ASSERT(LPC_CANAF->SFF_GRP_sa == 12 - offset);
  CAN_ASSERT(LPC_CANAF->EFF_sa == 20 - offset);
  CAN_ASSERT(LPC_CANAF->EFF_GRP_sa == 28 - offset);
  CAN_ASSERT(LPC_CANAF->ENDofTable == 36 - offset);
  for (i = 0; i < 10; i++) {
    printf("%2d: 0x%08X\n", (unsigned)i, (unsigned)LPC_CANAF_RAM->mask[i]);
  }

  /* Send a message defined in filter */
  printf("  Test filter messages\n");
  msg.frame = 0;
  msg.frame_fields.is_29bit = 0;
  msg.frame_fields.data_len = 8;
  msg.data.qword = 0x1122334455667788;

  /* Test reception of messages defined in the filter */
  printf("  Test message reception according to filter\n");
  msg.frame_fields.is_29bit = 0;
  can_test_msg(msg, 0x100, true); // standard id
  can_test_msg(msg, 0x110, true); // standard id
  can_test_msg(msg, 0x120, true); // standard id
  can_test_msg(msg, 0x130, true); // standard id
  can_test_msg(msg, 0x200, true); // Start of standard ID group
  can_test_msg(msg, 0x210, true); // Last of standard ID group
  can_test_msg(msg, 0x220, true); // Start of standard ID group
  can_test_msg(msg, 0x230, true); // Last of standard ID group

  msg.frame_fields.is_29bit = 1;
  can_test_msg(msg, 0x7500, true); // extended id
  can_test_msg(msg, 0x8500, true); // extended id
  can_test_msg(msg, 0xA000, true); // extended id group start
  can_test_msg(msg, 0xB000, true); // extended id group end

  printf("  Test messages that should not be received\n");
  /* Send a message not defined in filter */
  msg.frame_fields.is_29bit = 0;
  can_test_msg(msg, 0x0FF, false);
  can_test_msg(msg, 0x111, false);
  can_test_msg(msg, 0x131, false);
  can_test_msg(msg, 0x1FF, false);
  can_test_msg(msg, 0x211, false);
  can_test_msg(msg, 0x21f, false);
  can_test_msg(msg, 0x231, false);

  msg.frame_fields.is_29bit = 1;
  can_test_msg(msg, 0x7501, false);
  can_test_msg(msg, 0x8501, false);
  can_test_msg(msg, 0xA000 - 1, false);
  can_test_msg(msg, 0xB000 + 1, false);

  /* Test FullCAN */
  printf("  Test FullCAN\n");
  can__init(can1, 100, 5, 5, can__test_bufoff_cb, can__test_bufovr_cb);
  can__reset_bus(can1);
  id = 0x100;
  CAN_ASSERT(0 == can__fullcan_get_num_entries());

  CAN_ASSERT(
      can__fullcan_add_entry(can1, can__generate_standard_id(can1, id), can__generate_standard_id(can1, id + 1)));
  CAN_ASSERT(2 == can__fullcan_get_num_entries());
  CAN_ASSERT(LPC_CANAF->SFF_sa == 4);
  CAN_ASSERT(LPC_CANAF->SFF_GRP_sa == 4);
  CAN_ASSERT(LPC_CANAF->EFF_sa == 4);
  CAN_ASSERT(LPC_CANAF->EFF_GRP_sa == 4);
  CAN_ASSERT(LPC_CANAF->ENDofTable == 4);

  CAN_ASSERT(
      can__fullcan_add_entry(can1, can__generate_standard_id(can1, id + 2), can__generate_standard_id(can1, id + 3)));
  CAN_ASSERT(4 == can__fullcan_get_num_entries());
  CAN_ASSERT(LPC_CANAF->SFF_sa == 8);

  for (i = 0; i < 3; i++) {
    printf("%2i: 0x%08X\n", (unsigned)i, (unsigned)LPC_CANAF_RAM->mask[i]);
  }

  can__fullcan_msg_t *fc1 = can__fullcan_get_entry_ptr(can__generate_standard_id(can1, id));
  can__fullcan_msg_t *fc2 = can__fullcan_get_entry_ptr(can__generate_standard_id(can1, id + 1));
  can__fullcan_msg_t *fc3 = can__fullcan_get_entry_ptr(can__generate_standard_id(can1, id + 2));
  can__fullcan_msg_t *fc4 = can__fullcan_get_entry_ptr(can__generate_standard_id(can1, id + 3));
  CAN_ASSERT((LPC_CANAF_RAM_BASE + LPC_CANAF->SFF_sa) == (uint32_t)fc1);
  CAN_ASSERT((LPC_CANAF_RAM_BASE + LPC_CANAF->SFF_sa + 1 * sizeof(can__fullcan_msg_t)) == (uint32_t)fc2);
  CAN_ASSERT((LPC_CANAF_RAM_BASE + LPC_CANAF->SFF_sa + 2 * sizeof(can__fullcan_msg_t)) == (uint32_t)fc3);
  CAN_ASSERT((LPC_CANAF_RAM_BASE + LPC_CANAF->SFF_sa + 3 * sizeof(can__fullcan_msg_t)) == (uint32_t)fc4);

  can__fullcan_msg_t fc_temp;
  CAN_ASSERT(!can__fullcan_read_msg_copy(fc1, &fc_temp));
  CAN_ASSERT(!can__fullcan_read_msg_copy(fc2, &fc_temp));
  CAN_ASSERT(!can__fullcan_read_msg_copy(fc3, &fc_temp));
  CAN_ASSERT(!can__fullcan_read_msg_copy(fc4, &fc_temp));

  /* Send message, see if fullcan captures it */
  msg.frame = 0;
  msg.msg_id = id;
  msg.frame_fields.is_29bit = 0;
  msg.frame_fields.data_len = 8;

#define can_test_fullcan_msg(fc, msg_copy, id)                                                                         \
  do {                                                                                                                 \
    msg.msg_id = id;                                                                                                   \
    CAN_ASSERT(can__tx(can1, &msg, 0));                                                                                \
    CAN_ASSERT(!can__rx(can1, &msg, 10));                                                                              \
    CAN_ASSERT(can__fullcan_read_msg_copy(fc, &msg_copy));                                                             \
    CAN_ASSERT(fc->msg_id == id)                                                                                       \
  } while (0)
  can_test_fullcan_msg(fc1, fc_temp, id + 0);
  CAN_ASSERT(!can__fullcan_read_msg_copy(fc2, &fc_temp));
  can_test_fullcan_msg(fc2, fc_temp, id + 1);
  CAN_ASSERT(!can__fullcan_read_msg_copy(fc3, &fc_temp));
  can_test_fullcan_msg(fc3, fc_temp, id + 2);
  CAN_ASSERT(!can__fullcan_read_msg_copy(fc4, &fc_temp));
  can_test_fullcan_msg(fc4, fc_temp, id + 3);
  CAN_ASSERT(!can__fullcan_read_msg_copy(fc1, &fc_temp));

  printf("  \n--> All tests successful! <--\n");
  return true;
}
#endif
