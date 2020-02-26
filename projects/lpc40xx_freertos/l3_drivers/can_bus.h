/**
 * @file
 * @ingroup Drivers
 *
 * This is a simple CAN driver that optionally utilizes the FullCAN capability.
 * FullCAN is hardware filtering of the CAN messages and the configured messages
 * are stored directly into the CAN RAM (separate 2K RAM) without CPU intervention.
 *
 * FullCAN feature summary:
 *      - Messages go straight into dedicated RAM, SW never needs to be interrupted
 *      - Each FullCAN entry can generate an interrupt (up to 64 messages)
 *      - FullCAN entry can only be 11-bit NOT 29-bit type.
 *      - Note that FullCAN is for BOTH CANs.  If you wish to have separate message reads
 *        of individual CANs, you can enable non FullCAN filters because the implementation
 *        provides queues of separate CAN channels.
 *
 * When the CAN bus is initialized, we initially configure it to not accept any messages
 * until filtering or bypass option is used.  For the filtering, in addition to the FullCAN,
 * you can configure explicit 11-bit and 29-bit IDs (including groups) and only these
 * messages will be accepted.
 *
 * Note that if you send a message, and no other node acknowledges the message sent,
 * then the CAN BUS may enter "Bus off" state due to error(s).  You must correct
 * this situation and reset the CAN BUS.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * 8-byte structure accessible by 8-bit, 16-bit, 32-bit or as whole 64-bit
 * DO NOT CHANGE THIS STRUCTURE - it maps to the hardware
 */
typedef union {
  uint64_t qword;     ///< All 64-bits of data of a CAN message
  uint32_t dwords[2]; ///< dwords[0] = Byte 0-3,   dwords[1] = Byte 4-7
  uint16_t words[4];  ///<  words[0] = Byte 0-1 ... words[3] = Byte 6-7
  uint8_t bytes[8];   ///< 8 bytes of a CAN message
} can__data_t;

/**
 * Type definition of a CAN message with bit-fields (assuming little-endian machine)
 * DO NOT CHANGE THIS STRUCTURE - it maps to the hardware
 */
typedef struct {
  union {
    uint32_t frame; ///< 32-bit CAN frame aligned with frame_fields (bit members)
    struct {
      uint32_t : 16;         ///< Unused space
      uint32_t data_len : 4; ///< Data length
      uint32_t : 10;         ///< Unused space
      uint32_t is_rtr : 1;   ///< Message is an RTR type
      uint32_t is_29bit : 1; ///< If message ID is 29-bit type
    } frame_fields;
  };

  uint32_t msg_id;  ///< CAN Message ID (11-bit or 29-bit)
  can__data_t data; ///< CAN data
} __attribute__((__packed__)) can__msg_t;

/**
 * Typedef of a FullCAN message stored in memory
 * DO NOT CHANGE THIS STRUCTURE - it maps to the hardware
 */
typedef struct {
  struct {
    uint32_t msg_id : 11;  ///< 11-bit message-id
    uint32_t : 5;          ///< Unused
    uint32_t data_len : 4; ///< Message length
    uint32_t : 4;          ///< Unused
    uint32_t semphr : 2;   ///< Semaphore bits
    uint32_t : 4;          ///< Unused
    uint32_t rtr : 1;      ///< RTR message
    uint32_t : 1;          ///< Unused (FF bit, not applicable to FullCAN)
  };

  can__data_t data; ///< CAN data
} __attribute__((__packed__)) can__fullcan_msg_t;

/// The CAN BUS type
typedef enum {
  can1,    ///< CAN #1
  can2,    ///< CAN #2
  can_max, ///< Do not use or change
} can__num_e;

/**
 * CAN function pointer type
 * The 32-bit parameter is the value of the ICR register of CANbus
 */
typedef void (*can_void_func_t)(uint32_t);

/**
 * Initializes the CAN controller with the given baud-rate.
 * @warning This leaves the CAN BUS DISABLED!!!  The next steps are :
 * @code
 *   can__fullcan_add_entry(); // Optional
 *   can__setup_filter()();    // Optional
 *
 *   can__bypass_filter_accept_all_msgs(); // If you do not add a filter, just accept all messages
 *   can__reset_bus();
 * @endcode
 *
 * @param baudrate_kbps  The CAN Bus baud-rate, such as 100, 250, 500, 1000
 *                       Precise, external crystal should be used for higher than 100kbps
 * @param rxq_size  The size of the received messages queue
 * @param txq_size  The size of the transmit messages queue
 *
 * @param bus_off_cb  The callback function when CAN BUS enters BUS error state
 * @param data_ovr_cb The callback function when CAN BUS encounters data-overrun
 * @note  Both bus_off_cb and data_ovr_cb are optional.  BUS overrun callback should be used because if user doesn't
 *        correct this, the CAN BUS will never recover once the BUS error state is entered.
 *
 * @note  Each CAN BUS has separate receive and transmit queues
 */
bool can__init(can__num_e can, uint32_t baudrate_kbps, uint16_t rxq_size, uint16_t txq_size, can_void_func_t bus_off_cb,
               can_void_func_t data_ovr_cb);

/**
 * Receive a message of the given CAN BUS
 * @param timeout_ms  If FreeRTOS is running, the task will block until a message arrives.
 *                    Otherwise we will poll and wait this timeout to receive a message.
 * @returns true if message was captured within the given timeout.
 */
bool can__rx(can__num_e can, can__msg_t *can_message_ptr, uint32_t timeout_ms);

/**
 * Send a CAN message over the CAN BUS
 * @param timeout_ms  If FreeRTOS is running, the task will block for timeout_ms if the transmit queue is full
 * If FreeRTOS is not running the timeout is simply, ignored, and false is returned if all three HW buffers are full
 *
 * @return  If CAN message was either sent or queued, true is returned
 *          false is returned if transmit queue is full and timeout occurs while waiting for prior messages to be sent
 *
 * @code
 *   can__msg_t msg = {};
 *   msg.msg_id = 0x123;
 *   msg.frame_fields.is_29bit = 0;
 *   msg.frame_fields.data_len = 8;       // Send 8 bytes
 *   msg.data.qword = 0x1122334455667788; // Write all 8 bytes of data at once
 *   can__tx(can_1, &msg, portMAX_DELAY);
 * @endcode
 */
bool can__tx(can__num_e can, can__msg_t *can_message_ptr, uint32_t timeout_ms);

/** @{ CAN Bus Error and Reset API
 * If the CAN BUS encounters error(s), it may turn off, in which case no more
 * transmissions will take place.  This must be corrected by the user.
 */
bool can__is_bus_off(can__num_e can);
void can__reset_bus(can__num_e can);
/** @} */

/**
 * Enables CAN bypass mode to accept all messages on the bus.
 * Either CAN filters need to be setup or this method should be called to accept
 * CAN messages otherwise no messages will be capture at the CAN HW registers.
 *
 * @note  The filter is for BOTH CANs
 */
void can__bypass_filter_accept_all_msgs(void);

/** @{ Watermark and counter API */
uint16_t can__get_rx_watermark(can__num_e can); ///< FreeRTOS RX queue watermark
uint16_t can__get_tx_watermark(can__num_e can); ///< FreeRTOS TX queue watermark
uint16_t can__get_tx_count(can__num_e can);     ///< Number of messages written to the CAN HW
uint16_t can__get_rx_count(can__num_e can);     ///< Number of messages successfully queued from CAN interrupt

/**
 * Messages can be dropped out either if the receive queue is too small, or if there is no consumer or task
 * that dequeues the received messages quickly enough from the can__rx() API
 */
uint16_t can__get_rx_dropped_count(can__num_e can);
/** @} */

/* ---------------------------------------------------------------------------------------
 * Rest of the API is for filtering specific messages. Any messages not defined in the
 * acceptance filter are ignored, and will not be acknowledged by the CAN BUS.
 * --------------------------------------------------------------------------------------- */

/**
 * Standard ID HW filtering structure.  The single entries (not group) must be an even number count, so a second entry
 * can be used and should be marked disabled (disable bit set to 1) to make the count an even number.
 * DO NOT CHANGE THIS STRUCTURE - it maps to the hardware
 */
typedef union {
  struct {
    uint16_t id : 11;     ///< Standard 11-bit CAN ID
    uint16_t fc_intr : 1; ///< Message interrupt bit: ONLY USED FOR FULLCAN
    uint16_t disable : 1; ///< Set to 1 to disable the slot
    uint16_t can_num : 3; ///< CAN controller number (0=CAN1, 1=CAN2)
  };
  uint16_t raw;
} __attribute__((__packed__)) can_std_id_t;

/**
 * Standard ID group is nothing but a inclusive range of LOW and HIGH IDs
 * DO NOT CHANGE THIS STRUCTURE - it maps to the hardware
 */
typedef struct {     ///< CAN standard ID group
  can_std_id_t low;  ///< Low range
  can_std_id_t high; ///< High range
} can_std_grp_id_t;

/**
 * Extended ID entry is just the ID and the CAN BUS number
 * DO NOT CHANGE THIS STRUCTURE - it maps to the hardware
 */
typedef struct {
  uint32_t id : 29;     ///< Extended 29-bit CAN ID
  uint32_t can_num : 3; ///< CAN controller number (0=CAN1, 1=CAN2)
} __attribute__((__packed__)) can_ext_id_t;

/**
 * Extended ID group is nothing but a inclusive range of LOW and HIGH IDs
 * DO NOT CHANGE THIS STRUCTURE - it maps to the hardware
 */
typedef struct {     ///< CAN extended ID group
  can_ext_id_t low;  ///< Low range
  can_ext_id_t high; ///< High range
} can_ext_grp_id_t;

/**
 * @{
 * Generate and return the ID used to create the standard and extended id list member.  @see can__setup_filter()
 *
 * To generate a disabled slot, just pass 0xFFFF (id) as the message id, which will disable the message.
 * This is used to generate an empty slot to make an even number of entries as required by the standard id filter.
 */
can_std_id_t can__generate_standard_id(can__num_e can, uint16_t id);
can_ext_id_t can__generate_extended_id(can__num_e can, uint32_t id);
/** @} */

/**
 * Adds two FullCAN entries to the acceptance filter and enables the FullCAN reception.
 * The entries must be added in groups of 2.  If the second entry is not needed, simply
 * pass 0xFFFF to can__generate_standard_id() to generate a disabled entry.
 *
 * @returns true if successful.
 * @note Only 11-bit IDs can use FullCAN
 *
 * @warning This must be done BEFORE setting up other filters using can__setup_filter()
 * @warning CAN BUS should not be enabled to do this because the CAN Filter is put to
 *          OFF mode while the entry is added.
 *
 * @note TODO: Enabling fc_intr bit (FullCAN interrupts) is not yet supported
 */
bool can__fullcan_add_entry(can__num_e can, can_std_id_t id1, can_std_id_t id2);

/**
 * Once all the FulLCAN entries are added, and CAN filters are setup, you can get
 * the pointer in memory where the actual CAN message (FullCAN entry) is stored in memory.
 * @param fc_id The FullCAN entry originally passed to can__fullcan_add_entry()
 */
can__fullcan_msg_t *can__fullcan_get_entry_ptr(can_std_id_t fc_id);

/**
 * FullCAN entries may update at any time by the HW, so this method provides a means
 * to safely read the copy of the FullCAN message.
 * @param msg_copy_ptr  The same message returned from can__fullcan_get_entry_ptr()
 * @param fc_msg_ptr    The copy of the message you wish to read the data to
 * @returns true if a NEW message has been captured since the last call to this method.
 *          false if HW did not receive a new message
 */
bool can__fullcan_read_msg_copy(can__fullcan_msg_t *fc_msg_ptr, can__fullcan_msg_t *msg_copy_ptr);

/**
 * @returns the number of FullCAN entries being used
 */
uint8_t can__fullcan_get_num_entries(void);

/**
 * Enable CAN filter for BOTH CANs; hardware doesn't allow to enable for just ONE CAN controller.
 * @param std_id_list        List of 11-bit IDs to generate an ACK for (can be NULL)
 * @param sid_cnt            The size of the can_std_id_t array
 *
 * @param std_group_id_list  List of 11-bit ID groups to generate an ACK for (can be NULL)
 * @param sgp_cnt            The size of the can_std_grp_id_t array
 *
 * @param ext_id_list        List of 29-bit IDs to generate an ACK for (can be NULL)
 * @param eid_cnt            The size of the can_ext_id_t array
 *
 * @param ext_group_id_list List of 29-bit ID groups to generate an ACK for (can be NULL)
 * @param egp_cnt            The size of the can_ext_grp_id_t array
 *
 * @warning The list must be in ASCENDING order (lowest first, then highest).
 *          The hardware carries out its search taking for granted that the IDs are ordered
 *          from lowest to highest.  If filters are wrong, CAN ISR will enter a loop and
 *          your board will restart (due to watchdog).
 *
 * @warning CAN BUS should not be enabled to do this because the CAN Filter is put to
 *          OFF mode while the entry is added.
 *
 * @note    The CAN filter must be setup after the CAN controller(s) is initialized.
 * @note    The size entries is limited to 2K bytes.  Each entry size in bytes is :
 *              - can_std_id_t      : 2
 *              - can_std_grp_id_t  : 4
 *              - can_ext_id_t      : 4
 *              - can_ext_grp_id_t  : 8
 *
 * Here is sample code that enables HW filtering of selected CAN messages.
 * @note some messages are for CAN2 while most are for CAN1
 *
 * @code
 *      const can_std_id_t slist[]      = { can__generate_standard_id(can1, 0x100), can__generate_standard_id(can1,
 * 0x110),   // 2 entries can__generate_standard_id(can1, 0x120), can__generate_standard_id(can1, 0x130)    // 2 entries
 *                                        };
 *      const can_std_grp_id_t sglist[] = { {can__generate_standard_id(can1, 0x150), can__generate_standard_id(can1,
 * 0x200)}, // Group 1 {can__generate_standard_id(can2, 0x300), can__generate_standard_id(can2, 0x400)}  // Group 2
 *                                        };
 *      const can_ext_id_t *elist       = NULL; // Not used, so set it to NULL
 *      const can_ext_grp_id_t eglist[] = { {can__generate_extended_id(can1, 0x3500), can__generate_extended_id(can1,
 * 0x4500)} }; // Group 1
 *
 *      can__setup_filter(slist, 4, sglist, 2,
 *                        elist, 0, eglist, 1);
 * @endcode
 */
// clang-format off
bool can__setup_filter(const can_std_id_t *std_id_list,           uint16_t sid_cnt,
                       const can_std_grp_id_t *std_group_id_list, uint16_t sgp_cnt,
                       const can_ext_id_t *ext_id_list,           uint16_t eid_cnt,
                       const can_ext_grp_id_t *ext_group_id_list, uint16_t egp_cnt);
// clang-format on