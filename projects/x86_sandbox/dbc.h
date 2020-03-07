// clang-format off
// AUTO-GENERATED - DO NOT EDIT
// Generated from test.dbc
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifndef MIN_OF
#define MIN_OF(x,y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX_OF
#define MAX_OF(x,y) ((x) > (y) ? (x) : (y))
#endif

/**
 * Extern dependency to use dbc_encode_and_send_*() API
 * This is designed such that the generated code does not depend on your CAN driver
 * @param argument_from_dbc_encode_and_send is a pass through argument from dbc_encode_and_send_*()
 * @returns the value returned from is returned from dbc_encode_and_send_*()
 */
extern bool dbc_send_can_message(void * argument_from_dbc_encode_and_send, uint32_t message_id, const uint8_t bytes[8], uint8_t dlc);

/// Missing in Action (MIA) structure
typedef struct {
  uint32_t mia_counter; ///< Counter used to track MIA
} dbc_mia_info_t;

/**
 * dbc_encode_*() API returns this instance to indicate message ID and DLC that should be used
 * dbc_decode_*() API checks against the provided header to make sure we are decoding the right message
 */
typedef struct {
  uint32_t message_id;  ///< CAN bus message identification
  uint8_t message_dlc;  ///< Data Length Code of the CAN message
} dbc_message_header_t;

// Enumeration for DRIVER_HEARTBEAT_cmd
typedef enum {
  DRIVER_HEARTBEAT_cmd_REBOOT = 2,
  DRIVER_HEARTBEAT_cmd_SYNC = 1,
  DRIVER_HEARTBEAT_cmd_NOOP = 0,
} DRIVER_HEARTBEAT_cmd_e;

// Enumeration for DBC_TEST1_enum
typedef enum {
  DBC_TEST1_enum_val_two = 2,
  DBC_TEST1_enum_val_one = 1,
} DBC_TEST1_enum_e;

// Message headers containing CAN message IDs and their DLCs; @see dbc_message_header_t
static const dbc_message_header_t dbc_header_DBC_TEST1                           = {      500U, 8 };
static const dbc_message_header_t dbc_header_DBC_TEST2                           = {      501U, 8 };
static const dbc_message_header_t dbc_header_DBC_TEST3                           = {      502U, 8 };
static const dbc_message_header_t dbc_header_DRIVER_HEARTBEAT                    = {      100U, 1 };
static const dbc_message_header_t dbc_header_MOTOR_CMD                           = {      101U, 1 };
static const dbc_message_header_t dbc_header_MOTOR_STATUS                        = {      400U, 3 };
static const dbc_message_header_t dbc_header_SENSOR_SONARS                       = {      200U, 8 };
static const dbc_message_header_t dbc_header_DBC_TEST4                           = {      503U, 8 };

/**
 * DBC_TEST1: Sent by 'IO'
 *   - Expected every 100 ms
 */
typedef struct {
  dbc_mia_info_t mia_info;

  uint8_t DBC_TEST1_unsigned1;
  uint8_t DBC_TEST1_unsigned_minmax;
  DBC_TEST1_enum_e DBC_TEST1_enum;
  float DBC_TEST1_float;
  float DBC_TEST1_float_signed;
} dbc_DBC_TEST1_s;

/**
 * DBC_TEST2: Sent by 'IO'
 */
typedef struct {
  dbc_mia_info_t mia_info;

  int16_t DBC_TEST2_real_signed1;
  int32_t DBC_TEST2_real_signed2;
  int8_t DBC_TEST2_signed;
  int16_t DBC_TEST2_signed_minmax;
} dbc_DBC_TEST2_s;

/**
 * DBC_TEST3: Sent by 'IO'
 */
typedef struct {
  dbc_mia_info_t mia_info;

  int8_t DBC_TEST3_real_signed1;
  int16_t DBC_TEST3_real_signed2;
} dbc_DBC_TEST3_s;

/**
 * DRIVER_HEARTBEAT: Sent by 'DRIVER'
 *   - Sync message used to synchronize the controllers
 *   - Expected every 1000 ms
 */
typedef struct {
  dbc_mia_info_t mia_info;

  DRIVER_HEARTBEAT_cmd_e DRIVER_HEARTBEAT_cmd;
} dbc_DRIVER_HEARTBEAT_s;

/**
 * MOTOR_CMD: Sent by 'DRIVER'
 *   - Expected every 100 ms
 */
typedef struct {
  dbc_mia_info_t mia_info;

  int8_t MOTOR_CMD_steer;
  uint8_t MOTOR_CMD_drive;
} dbc_MOTOR_CMD_s;

/**
 * MOTOR_STATUS: Sent by 'MOTOR'
 *   - Expected every 100 ms
 */
typedef struct {
  dbc_mia_info_t mia_info;

  uint8_t MOTOR_STATUS_wheel_error;
  float MOTOR_STATUS_speed_kph; // unit: kph
} dbc_MOTOR_STATUS_s;

/**
 * SENSOR_SONARS: Sent by 'SENSOR'
 *   - Expected every 100 ms
 */
typedef struct {
  dbc_mia_info_t mia_info;

  uint8_t SENSOR_SONARS_mux; // Non-muxed signal
  uint16_t SENSOR_SONARS_err_count; // Non-muxed signal

  float SENSOR_SONARS_left; // M0
  float SENSOR_SONARS_middle; // M0
  float SENSOR_SONARS_right; // M0
  float SENSOR_SONARS_rear; // M0

  float SENSOR_SONARS_no_filt_left; // M1
  float SENSOR_SONARS_no_filt_middle; // M1
  float SENSOR_SONARS_no_filt_right; // M1
  float SENSOR_SONARS_no_filt_rear; // M1
} dbc_SENSOR_SONARS_s;

/**
 * DBC_TEST4: Sent by 'IO'
 */
typedef struct {
  dbc_mia_info_t mia_info;

  int32_t DBC_TEST4_real_signed1;
  int32_t DBC_TEST4_real_signed2;
  uint16_t DBC_TEST4_real_overflow;
} dbc_DBC_TEST4_s;

// -----------------------------------------------------------------------------
// When a message's MIA counter reaches this value
// corresponding MIA replacements occur
// -----------------------------------------------------------------------------
extern const uint32_t dbc_mia_threshold_DBC_TEST1;
extern const uint32_t dbc_mia_threshold_DBC_TEST2;
extern const uint32_t dbc_mia_threshold_DBC_TEST3;
extern const uint32_t dbc_mia_threshold_DRIVER_HEARTBEAT;
extern const uint32_t dbc_mia_threshold_MOTOR_CMD;
extern const uint32_t dbc_mia_threshold_MOTOR_STATUS;
extern const uint32_t dbc_mia_threshold_SENSOR_SONARS;
extern const uint32_t dbc_mia_threshold_DBC_TEST4;

// -----------------------------------------------------------------------------
// User must define these externed instances in their code to use MIA functions
// These are copied during dbc_service_mia_*() when message MIA timeout occurs
// -----------------------------------------------------------------------------
extern const dbc_DBC_TEST1_s            dbc_mia_replacement_DBC_TEST1; // Suggested MIA threshold: (3*100)
extern const dbc_DBC_TEST2_s            dbc_mia_replacement_DBC_TEST2;
extern const dbc_DBC_TEST3_s            dbc_mia_replacement_DBC_TEST3;
extern const dbc_DRIVER_HEARTBEAT_s     dbc_mia_replacement_DRIVER_HEARTBEAT; // Suggested MIA threshold: (3*1000)
extern const dbc_MOTOR_CMD_s            dbc_mia_replacement_MOTOR_CMD; // Suggested MIA threshold: (3*100)
extern const dbc_MOTOR_STATUS_s         dbc_mia_replacement_MOTOR_STATUS; // Suggested MIA threshold: (3*100)
extern const dbc_SENSOR_SONARS_s        dbc_mia_replacement_SENSOR_SONARS; // Suggested MIA threshold: (3*100)
extern const dbc_DBC_TEST4_s            dbc_mia_replacement_DBC_TEST4;


/**
 * Encode to transmit DBC_TEST1:
 *   Transmitter: 'IO' with message ID 500 composed of 8 bytes
 */
static inline dbc_message_header_t dbc_encode_DBC_TEST1(uint8_t bytes[8], const dbc_DBC_TEST1_s *message) {
  uint64_t raw = 0;
  memset(bytes, 0, 8);

  // Encode to raw 8-bit signal with scale=1
  raw = ((uint64_t)((((int64_t)message->DBC_TEST1_unsigned1)))) & 0xff;
  bytes[0] |= (((uint8_t)(raw >>  0) & 0xff)     ); // 8 bits at B0

  // Encode to raw 8-bit signal with scale=1 within range of [0 -> 100]
  raw = ((uint64_t)(((MAX_OF(MIN_OF((int64_t)message->DBC_TEST1_unsigned_minmax,100),0))))) & 0xff;
  bytes[1] |= (((uint8_t)(raw >>  0) & 0xff)     ); // 8 bits at B8

  // Encode to raw 8-bit signal with scale=1
  raw = ((uint64_t)((((int64_t)message->DBC_TEST1_enum)))) & 0xff;
  bytes[4] |= (((uint8_t)(raw >>  0) & 0xff)     ); // 8 bits at B32

  // Encode to raw 8-bit signal with scale=0.5
  raw = ((uint64_t)(((message->DBC_TEST1_float) / 0.5f) + 0.5f)) & 0xff;
  bytes[5] |= (((uint8_t)(raw >>  0) & 0xff)     ); // 8 bits at B40

  // Encode to raw 16-bit signal with scale=0.001 within range of [0 -> 65.535]
  raw = ((uint64_t)(((MAX_OF(MIN_OF(message->DBC_TEST1_float_signed,65.535f),0.0f)) / 0.001f) + 0.5f)) & 0xffff;
  bytes[6] |= (((uint8_t)(raw >>  0) & 0xff)     ); // 8 bits at B48
  bytes[7] |= (((uint8_t)(raw >>  8) & 0xff)     ); // 8 bits at B56

  return dbc_header_DBC_TEST1;
}

/// @see dbc_encode_DBC_TEST1(); this is its variant to encode and call dbc_send_can_message() to send the message
static inline bool dbc_encode_and_send_DBC_TEST1(void *argument_for_dbc_send_can_message, const dbc_DBC_TEST1_s *message) {
  uint8_t bytes[8];
  const dbc_message_header_t header = dbc_encode_DBC_TEST1(bytes, message);
  return dbc_send_can_message(argument_for_dbc_send_can_message, header.message_id, bytes, header.message_dlc);
}

/**
 * Encode to transmit DBC_TEST2:
 *   Transmitter: 'IO' with message ID 501 composed of 8 bytes
 */
static inline dbc_message_header_t dbc_encode_DBC_TEST2(uint8_t bytes[8], const dbc_DBC_TEST2_s *message) {
  uint64_t raw = 0;
  memset(bytes, 0, 8);

  // Encode to raw 12-bit SIGNED signal with scale=1
  raw = ((uint64_t)((((int64_t)message->DBC_TEST2_real_signed1)))) & 0xfff;
  bytes[0] |= (((uint8_t)(raw >>  0) & 0x03) << 6); // 2 bits at B6
  bytes[1] |= (((uint8_t)(raw >>  2) & 0xff)     ); // 8 bits at B8
  bytes[2] |= (((uint8_t)(raw >> 10) & 0x03)     ); // 2 bits at B16

  // Encode to raw 18-bit SIGNED signal with scale=1
  raw = ((uint64_t)((((int64_t)message->DBC_TEST2_real_signed2)))) & 0x3ffff;
  bytes[2] |= (((uint8_t)(raw >>  0) & 0x3f) << 2); // 6 bits at B18
  bytes[3] |= (((uint8_t)(raw >>  6) & 0xff)     ); // 8 bits at B24
  bytes[4] |= (((uint8_t)(raw >> 14) & 0x0f)     ); // 4 bits at B32

  // Encode to raw 8-bit signal with scale=1 and offset=-128
  raw = ((uint64_t)((((int64_t)message->DBC_TEST2_signed - (-128))))) & 0xff;
  bytes[4] |= (((uint8_t)(raw >>  0) & 0x0f) << 4); // 4 bits at B36
  bytes[5] |= (((uint8_t)(raw >>  4) & 0x0f)     ); // 4 bits at B40

  // Encode to raw 16-bit signal with scale=1 and offset=-32768 within range of [-32768 -> 32767]
  raw = ((uint64_t)(((MAX_OF(MIN_OF((int64_t)message->DBC_TEST2_signed_minmax,32767),-32768) - (-32768))))) & 0xffff;
  bytes[5] |= (((uint8_t)(raw >>  0) & 0x0f) << 4); // 4 bits at B44
  bytes[6] |= (((uint8_t)(raw >>  4) & 0xff)     ); // 8 bits at B48
  bytes[7] |= (((uint8_t)(raw >> 12) & 0x0f)     ); // 4 bits at B56

  return dbc_header_DBC_TEST2;
}

/// @see dbc_encode_DBC_TEST2(); this is its variant to encode and call dbc_send_can_message() to send the message
static inline bool dbc_encode_and_send_DBC_TEST2(void *argument_for_dbc_send_can_message, const dbc_DBC_TEST2_s *message) {
  uint8_t bytes[8];
  const dbc_message_header_t header = dbc_encode_DBC_TEST2(bytes, message);
  return dbc_send_can_message(argument_for_dbc_send_can_message, header.message_id, bytes, header.message_dlc);
}

/**
 * Encode to transmit DBC_TEST3:
 *   Transmitter: 'IO' with message ID 502 composed of 8 bytes
 */
static inline dbc_message_header_t dbc_encode_DBC_TEST3(uint8_t bytes[8], const dbc_DBC_TEST3_s *message) {
  uint64_t raw = 0;
  memset(bytes, 0, 8);

  // Encode to raw 4-bit SIGNED signal with scale=1 and offset=-5
  raw = ((uint64_t)((((int64_t)message->DBC_TEST3_real_signed1 - (-5))))) & 0x0f;
  bytes[0] |= (((uint8_t)(raw >>  0) & 0x0f) << 2); // 4 bits at B2

  // Encode to raw 8-bit SIGNED signal with scale=1 and offset=5
  raw = ((uint64_t)((((int64_t)message->DBC_TEST3_real_signed2 - (5))))) & 0xff;
  bytes[1] |= (((uint8_t)(raw >>  0) & 0xff)     ); // 8 bits at B8

  return dbc_header_DBC_TEST3;
}

/// @see dbc_encode_DBC_TEST3(); this is its variant to encode and call dbc_send_can_message() to send the message
static inline bool dbc_encode_and_send_DBC_TEST3(void *argument_for_dbc_send_can_message, const dbc_DBC_TEST3_s *message) {
  uint8_t bytes[8];
  const dbc_message_header_t header = dbc_encode_DBC_TEST3(bytes, message);
  return dbc_send_can_message(argument_for_dbc_send_can_message, header.message_id, bytes, header.message_dlc);
}

/**
 * Encode to transmit DRIVER_HEARTBEAT:
 *   Transmitter: 'DRIVER' with message ID 100 composed of 1 bytes
 */
static inline dbc_message_header_t dbc_encode_DRIVER_HEARTBEAT(uint8_t bytes[8], const dbc_DRIVER_HEARTBEAT_s *message) {
  uint64_t raw = 0;
  memset(bytes, 0, 8);

  // Encode to raw 8-bit signal with scale=1
  raw = ((uint64_t)((((int64_t)message->DRIVER_HEARTBEAT_cmd)))) & 0xff;
  bytes[0] |= (((uint8_t)(raw >>  0) & 0xff)     ); // 8 bits at B0

  return dbc_header_DRIVER_HEARTBEAT;
}

/// @see dbc_encode_DRIVER_HEARTBEAT(); this is its variant to encode and call dbc_send_can_message() to send the message
static inline bool dbc_encode_and_send_DRIVER_HEARTBEAT(void *argument_for_dbc_send_can_message, const dbc_DRIVER_HEARTBEAT_s *message) {
  uint8_t bytes[8];
  const dbc_message_header_t header = dbc_encode_DRIVER_HEARTBEAT(bytes, message);
  return dbc_send_can_message(argument_for_dbc_send_can_message, header.message_id, bytes, header.message_dlc);
}

/**
 * Encode to transmit MOTOR_CMD:
 *   Transmitter: 'DRIVER' with message ID 101 composed of 1 bytes
 */
static inline dbc_message_header_t dbc_encode_MOTOR_CMD(uint8_t bytes[8], const dbc_MOTOR_CMD_s *message) {
  uint64_t raw = 0;
  memset(bytes, 0, 8);

  // Encode to raw 4-bit signal with scale=1 and offset=-5 within range of [-5 -> 5]
  raw = ((uint64_t)(((MAX_OF(MIN_OF((int64_t)message->MOTOR_CMD_steer,5),-5) - (-5))))) & 0x0f;
  bytes[0] |= (((uint8_t)(raw >>  0) & 0x0f)     ); // 4 bits at B0

  // Encode to raw 4-bit signal with scale=1 within range of [0 -> 9]
  raw = ((uint64_t)(((MAX_OF(MIN_OF((int64_t)message->MOTOR_CMD_drive,9),0))))) & 0x0f;
  bytes[0] |= (((uint8_t)(raw >>  0) & 0x0f) << 4); // 4 bits at B4

  return dbc_header_MOTOR_CMD;
}

/// @see dbc_encode_MOTOR_CMD(); this is its variant to encode and call dbc_send_can_message() to send the message
static inline bool dbc_encode_and_send_MOTOR_CMD(void *argument_for_dbc_send_can_message, const dbc_MOTOR_CMD_s *message) {
  uint8_t bytes[8];
  const dbc_message_header_t header = dbc_encode_MOTOR_CMD(bytes, message);
  return dbc_send_can_message(argument_for_dbc_send_can_message, header.message_id, bytes, header.message_dlc);
}

/**
 * Encode to transmit MOTOR_STATUS:
 *   Transmitter: 'MOTOR' with message ID 400 composed of 3 bytes
 */
static inline dbc_message_header_t dbc_encode_MOTOR_STATUS(uint8_t bytes[8], const dbc_MOTOR_STATUS_s *message) {
  uint64_t raw = 0;
  memset(bytes, 0, 8);

  // Encode to raw 1-bit signal with scale=1
  raw = ((uint64_t)((((int64_t)message->MOTOR_STATUS_wheel_error)))) & 0x01;
  bytes[0] |= (((uint8_t)(raw >>  0) & 0x01)     ); // 1 bits at B0

  // Encode to raw 16-bit signal with scale=0.001
  raw = ((uint64_t)(((message->MOTOR_STATUS_speed_kph) / 0.001f) + 0.5f)) & 0xffff;
  bytes[1] |= (((uint8_t)(raw >>  0) & 0xff)     ); // 8 bits at B8
  bytes[2] |= (((uint8_t)(raw >>  8) & 0xff)     ); // 8 bits at B16

  return dbc_header_MOTOR_STATUS;
}

/// @see dbc_encode_MOTOR_STATUS(); this is its variant to encode and call dbc_send_can_message() to send the message
static inline bool dbc_encode_and_send_MOTOR_STATUS(void *argument_for_dbc_send_can_message, const dbc_MOTOR_STATUS_s *message) {
  uint8_t bytes[8];
  const dbc_message_header_t header = dbc_encode_MOTOR_STATUS(bytes, message);
  return dbc_send_can_message(argument_for_dbc_send_can_message, header.message_id, bytes, header.message_dlc);
}

/**
 * Encode to transmit SENSOR_SONARS:
 *   Transmitter: 'SENSOR' with message ID 200 composed of 8 bytes
 */
static inline dbc_message_header_t dbc_encode_SENSOR_SONARS(uint8_t bytes[8], const dbc_SENSOR_SONARS_s *message) {
  uint64_t raw = 0;
  memset(bytes, 0, 8);

  // Multiplexed signals are not handled yet
  (void)raw;

  return dbc_header_SENSOR_SONARS;
}

/// @see dbc_encode_SENSOR_SONARS(); this is its variant to encode and call dbc_send_can_message() to send the message
static inline bool dbc_encode_and_send_SENSOR_SONARS(void *argument_for_dbc_send_can_message, const dbc_SENSOR_SONARS_s *message) {
  uint8_t bytes[8];
  const dbc_message_header_t header = dbc_encode_SENSOR_SONARS(bytes, message);
  return dbc_send_can_message(argument_for_dbc_send_can_message, header.message_id, bytes, header.message_dlc);
}

/**
 * Encode to transmit DBC_TEST4:
 *   Transmitter: 'IO' with message ID 503 composed of 8 bytes
 */
static inline dbc_message_header_t dbc_encode_DBC_TEST4(uint8_t bytes[8], const dbc_DBC_TEST4_s *message) {
  uint64_t raw = 0;
  memset(bytes, 0, 8);

  // Encode to raw 32-bit SIGNED signal with scale=1
  raw = ((uint64_t)((((int64_t)message->DBC_TEST4_real_signed1)))) & 0xffffffff;
  bytes[0] |= (((uint8_t)(raw >>  0) & 0xff)     ); // 8 bits at B0
  bytes[1] |= (((uint8_t)(raw >>  8) & 0xff)     ); // 8 bits at B8
  bytes[2] |= (((uint8_t)(raw >> 16) & 0xff)     ); // 8 bits at B16
  bytes[3] |= (((uint8_t)(raw >> 24) & 0xff)     ); // 8 bits at B24

  // Encode to raw 16-bit SIGNED signal with scale=1 within range of [-32768 -> 32767]
  raw = ((uint64_t)(((MAX_OF(MIN_OF((int64_t)message->DBC_TEST4_real_signed2,32767),-32768))))) & 0xffff;
  bytes[4] |= (((uint8_t)(raw >>  0) & 0xff)     ); // 8 bits at B32
  bytes[5] |= (((uint8_t)(raw >>  8) & 0xff)     ); // 8 bits at B40

  // Encode to raw 16-bit signal with scale=1
  raw = ((uint64_t)((((int64_t)message->DBC_TEST4_real_overflow)))) & 0xffff;
  bytes[6] |= (((uint8_t)(raw >>  0) & 0xff)     ); // 8 bits at B48
  bytes[7] |= (((uint8_t)(raw >>  8) & 0xff)     ); // 8 bits at B56

  return dbc_header_DBC_TEST4;
}

/// @see dbc_encode_DBC_TEST4(); this is its variant to encode and call dbc_send_can_message() to send the message
static inline bool dbc_encode_and_send_DBC_TEST4(void *argument_for_dbc_send_can_message, const dbc_DBC_TEST4_s *message) {
  uint8_t bytes[8];
  const dbc_message_header_t header = dbc_encode_DBC_TEST4(bytes, message);
  return dbc_send_can_message(argument_for_dbc_send_can_message, header.message_id, bytes, header.message_dlc);
}

/**
 * Decode received message DBC_TEST1: Sent by IO
 *   Provided a dbc_message_header_t, this will attempt to decode the received message and return true upon success
 */
static inline bool dbc_decode_DBC_TEST1(dbc_DBC_TEST1_s *message, const dbc_message_header_t header, const uint8_t bytes[8]) {
  const bool success = true;

  if ((header.message_id != dbc_header_DBC_TEST1.message_id) || (header.message_dlc != dbc_header_DBC_TEST1.message_dlc)) {
    return !success;
  }

  uint64_t raw = 0;
  raw  = ((uint64_t)((bytes[0]))); // 8 bits from B0
  message->DBC_TEST1_unsigned1 = ((raw));

  raw  = ((uint64_t)((bytes[1]))); // 8 bits from B8
  message->DBC_TEST1_unsigned_minmax = ((raw));

  raw  = ((uint64_t)((bytes[4]))); // 8 bits from B32
  message->DBC_TEST1_enum = (DBC_TEST1_enum_e)((raw));

  raw  = ((uint64_t)((bytes[5]))); // 8 bits from B40
  message->DBC_TEST1_float = ((raw * 0.5f));

  raw  = ((uint64_t)((bytes[6]))); // 8 bits from B48
  raw |= ((uint64_t)((bytes[7]))) << 8; // 8 bits from B56
  message->DBC_TEST1_float_signed = ((raw * 0.001f));

  message->mia_info.mia_counter = 0;
  return success;
}

/**
 * Decode received message DBC_TEST2: Sent by IO
 *   Provided a dbc_message_header_t, this will attempt to decode the received message and return true upon success
 */
static inline bool dbc_decode_DBC_TEST2(dbc_DBC_TEST2_s *message, const dbc_message_header_t header, const uint8_t bytes[8]) {
  const bool success = true;

  if ((header.message_id != dbc_header_DBC_TEST2.message_id) || (header.message_dlc != dbc_header_DBC_TEST2.message_dlc)) {
    return !success;
  }

  uint64_t raw = 0;
  raw  = ((uint64_t)((bytes[0] >> 6) & 0x03)); // 2 bits from B6
  raw |= ((uint64_t)((bytes[1]))) << 2; // 8 bits from B8
  raw |= ((uint64_t)((bytes[2]) & 0x03)) << 10; // 2 bits from B16
  if (raw & (1 << 11)) { // Check signed bit of the raw DBC signal and sign extend from 'raw'
    message->DBC_TEST2_real_signed1 = ((((UINT32_MAX << 11) | raw)));
  } else {
    message->DBC_TEST2_real_signed1 = ((raw));
  }

  raw  = ((uint64_t)((bytes[2] >> 2) & 0x3f)); // 6 bits from B18
  raw |= ((uint64_t)((bytes[3]))) << 6; // 8 bits from B24
  raw |= ((uint64_t)((bytes[4]) & 0x0f)) << 14; // 4 bits from B32
  if (raw & (1 << 17)) { // Check signed bit of the raw DBC signal and sign extend from 'raw'
    message->DBC_TEST2_real_signed2 = ((((UINT32_MAX << 17) | raw)));
  } else {
    message->DBC_TEST2_real_signed2 = ((raw));
  }

  raw  = ((uint64_t)((bytes[4] >> 4) & 0x0f)); // 4 bits from B36
  raw |= ((uint64_t)((bytes[5]) & 0x0f)) << 4; // 4 bits from B40
  message->DBC_TEST2_signed = ((raw) + (-128));

  raw  = ((uint64_t)((bytes[5] >> 4) & 0x0f)); // 4 bits from B44
  raw |= ((uint64_t)((bytes[6]))) << 4; // 8 bits from B48
  raw |= ((uint64_t)((bytes[7]) & 0x0f)) << 12; // 4 bits from B56
  message->DBC_TEST2_signed_minmax = ((raw) + (-32768));

  message->mia_info.mia_counter = 0;
  return success;
}

/**
 * Decode received message DBC_TEST3: Sent by IO
 *   Provided a dbc_message_header_t, this will attempt to decode the received message and return true upon success
 */
static inline bool dbc_decode_DBC_TEST3(dbc_DBC_TEST3_s *message, const dbc_message_header_t header, const uint8_t bytes[8]) {
  const bool success = true;

  if ((header.message_id != dbc_header_DBC_TEST3.message_id) || (header.message_dlc != dbc_header_DBC_TEST3.message_dlc)) {
    return !success;
  }

  uint64_t raw = 0;
  raw  = ((uint64_t)((bytes[0] >> 2) & 0x0f)); // 4 bits from B2
  if (raw & (1 << 3)) { // Check signed bit of the raw DBC signal and sign extend from 'raw'
    message->DBC_TEST3_real_signed1 = ((((UINT32_MAX << 3) | raw)) + (-5));
  } else {
    message->DBC_TEST3_real_signed1 = ((raw) + (-5));
  }

  raw  = ((uint64_t)((bytes[1]))); // 8 bits from B8
  if (raw & (1 << 7)) { // Check signed bit of the raw DBC signal and sign extend from 'raw'
    message->DBC_TEST3_real_signed2 = ((((UINT32_MAX << 7) | raw)) + (5));
  } else {
    message->DBC_TEST3_real_signed2 = ((raw) + (5));
  }

  message->mia_info.mia_counter = 0;
  return success;
}

/**
 * Decode received message DRIVER_HEARTBEAT: Sent by DRIVER
 *   Provided a dbc_message_header_t, this will attempt to decode the received message and return true upon success
 */
static inline bool dbc_decode_DRIVER_HEARTBEAT(dbc_DRIVER_HEARTBEAT_s *message, const dbc_message_header_t header, const uint8_t bytes[8]) {
  const bool success = true;

  if ((header.message_id != dbc_header_DRIVER_HEARTBEAT.message_id) || (header.message_dlc != dbc_header_DRIVER_HEARTBEAT.message_dlc)) {
    return !success;
  }

  uint64_t raw = 0;
  raw  = ((uint64_t)((bytes[0]))); // 8 bits from B0
  message->DRIVER_HEARTBEAT_cmd = (DRIVER_HEARTBEAT_cmd_e)((raw));

  message->mia_info.mia_counter = 0;
  return success;
}

/**
 * Decode received message MOTOR_CMD: Sent by DRIVER
 *   Provided a dbc_message_header_t, this will attempt to decode the received message and return true upon success
 */
static inline bool dbc_decode_MOTOR_CMD(dbc_MOTOR_CMD_s *message, const dbc_message_header_t header, const uint8_t bytes[8]) {
  const bool success = true;

  if ((header.message_id != dbc_header_MOTOR_CMD.message_id) || (header.message_dlc != dbc_header_MOTOR_CMD.message_dlc)) {
    return !success;
  }

  uint64_t raw = 0;
  raw  = ((uint64_t)((bytes[0]) & 0x0f)); // 4 bits from B0
  message->MOTOR_CMD_steer = ((raw) + (-5));

  raw  = ((uint64_t)((bytes[0] >> 4) & 0x0f)); // 4 bits from B4
  message->MOTOR_CMD_drive = ((raw));

  message->mia_info.mia_counter = 0;
  return success;
}

/**
 * Decode received message MOTOR_STATUS: Sent by MOTOR
 *   Provided a dbc_message_header_t, this will attempt to decode the received message and return true upon success
 */
static inline bool dbc_decode_MOTOR_STATUS(dbc_MOTOR_STATUS_s *message, const dbc_message_header_t header, const uint8_t bytes[8]) {
  const bool success = true;

  if ((header.message_id != dbc_header_MOTOR_STATUS.message_id) || (header.message_dlc != dbc_header_MOTOR_STATUS.message_dlc)) {
    return !success;
  }

  uint64_t raw = 0;
  raw  = ((uint64_t)((bytes[0]) & 0x01)); // 1 bits from B0
  message->MOTOR_STATUS_wheel_error = ((raw));

  raw  = ((uint64_t)((bytes[1]))); // 8 bits from B8
  raw |= ((uint64_t)((bytes[2]))) << 8; // 8 bits from B16
  message->MOTOR_STATUS_speed_kph = ((raw * 0.001f));

  message->mia_info.mia_counter = 0;
  return success;
}

/**
 * Decode received message SENSOR_SONARS: Sent by SENSOR
 *   Provided a dbc_message_header_t, this will attempt to decode the received message and return true upon success
 */
static inline bool dbc_decode_SENSOR_SONARS(dbc_SENSOR_SONARS_s *message, const dbc_message_header_t header, const uint8_t bytes[8]) {
  const bool success = true;

  if ((header.message_id != dbc_header_SENSOR_SONARS.message_id) || (header.message_dlc != dbc_header_SENSOR_SONARS.message_dlc)) {
    return !success;
  }

  uint64_t raw = 0;
  // Multiplexed signals are not handled yet
  (void)raw;

  message->mia_info.mia_counter = 0;
  return success;
}

/**
 * Decode received message DBC_TEST4: Sent by IO
 *   Provided a dbc_message_header_t, this will attempt to decode the received message and return true upon success
 */
static inline bool dbc_decode_DBC_TEST4(dbc_DBC_TEST4_s *message, const dbc_message_header_t header, const uint8_t bytes[8]) {
  const bool success = true;

  if ((header.message_id != dbc_header_DBC_TEST4.message_id) || (header.message_dlc != dbc_header_DBC_TEST4.message_dlc)) {
    return !success;
  }

  uint64_t raw = 0;
  raw  = ((uint64_t)((bytes[0]))); // 8 bits from B0
  raw |= ((uint64_t)((bytes[1]))) << 8; // 8 bits from B8
  raw |= ((uint64_t)((bytes[2]))) << 16; // 8 bits from B16
  raw |= ((uint64_t)((bytes[3]))) << 24; // 8 bits from B24
  if (raw & (1 << 31)) { // Check signed bit of the raw DBC signal and sign extend from 'raw'
    message->DBC_TEST4_real_signed1 = ((((UINT32_MAX << 31) | raw)));
  } else {
    message->DBC_TEST4_real_signed1 = ((raw));
  }

  raw  = ((uint64_t)((bytes[4]))); // 8 bits from B32
  raw |= ((uint64_t)((bytes[5]))) << 8; // 8 bits from B40
  if (raw & (1 << 15)) { // Check signed bit of the raw DBC signal and sign extend from 'raw'
    message->DBC_TEST4_real_signed2 = ((((UINT32_MAX << 15) | raw)));
  } else {
    message->DBC_TEST4_real_signed2 = ((raw));
  }

  raw  = ((uint64_t)((bytes[6]))); // 8 bits from B48
  raw |= ((uint64_t)((bytes[7]))) << 8; // 8 bits from B56
  message->DBC_TEST4_real_overflow = ((raw));

  message->mia_info.mia_counter = 0;
  return success;
}

// Do not use this function
static inline bool dbc_service_mia_for(dbc_mia_info_t *mia_info, const uint32_t increment_mia_by, const uint32_t threshold) {
  bool message_just_entered_mia = false;

  if (mia_info->mia_counter >= threshold) {
    // Message is already MIA
  } else {
    mia_info->mia_counter += increment_mia_by;
    message_just_entered_mia = (mia_info->mia_counter >= threshold);
  }

  return message_just_entered_mia;
}

static inline bool dbc_service_mia_DBC_TEST1(dbc_DBC_TEST1_s *message, const uint32_t increment_mia_by) {
  const bool message_just_entered_mia = dbc_service_mia_for(&(message->mia_info), increment_mia_by, dbc_mia_threshold_DBC_TEST1);

  if (message_just_entered_mia) {
    const dbc_mia_info_t previous_mia = message->mia_info;
    *message = dbc_mia_replacement_DBC_TEST1;
    message->mia_info = previous_mia;
  }

  return message_just_entered_mia;
}

static inline bool dbc_service_mia_DBC_TEST2(dbc_DBC_TEST2_s *message, const uint32_t increment_mia_by) {
  const bool message_just_entered_mia = dbc_service_mia_for(&(message->mia_info), increment_mia_by, dbc_mia_threshold_DBC_TEST2);

  if (message_just_entered_mia) {
    const dbc_mia_info_t previous_mia = message->mia_info;
    *message = dbc_mia_replacement_DBC_TEST2;
    message->mia_info = previous_mia;
  }

  return message_just_entered_mia;
}

static inline bool dbc_service_mia_DBC_TEST3(dbc_DBC_TEST3_s *message, const uint32_t increment_mia_by) {
  const bool message_just_entered_mia = dbc_service_mia_for(&(message->mia_info), increment_mia_by, dbc_mia_threshold_DBC_TEST3);

  if (message_just_entered_mia) {
    const dbc_mia_info_t previous_mia = message->mia_info;
    *message = dbc_mia_replacement_DBC_TEST3;
    message->mia_info = previous_mia;
  }

  return message_just_entered_mia;
}

static inline bool dbc_service_mia_DRIVER_HEARTBEAT(dbc_DRIVER_HEARTBEAT_s *message, const uint32_t increment_mia_by) {
  const bool message_just_entered_mia = dbc_service_mia_for(&(message->mia_info), increment_mia_by, dbc_mia_threshold_DRIVER_HEARTBEAT);

  if (message_just_entered_mia) {
    const dbc_mia_info_t previous_mia = message->mia_info;
    *message = dbc_mia_replacement_DRIVER_HEARTBEAT;
    message->mia_info = previous_mia;
  }

  return message_just_entered_mia;
}

static inline bool dbc_service_mia_MOTOR_CMD(dbc_MOTOR_CMD_s *message, const uint32_t increment_mia_by) {
  const bool message_just_entered_mia = dbc_service_mia_for(&(message->mia_info), increment_mia_by, dbc_mia_threshold_MOTOR_CMD);

  if (message_just_entered_mia) {
    const dbc_mia_info_t previous_mia = message->mia_info;
    *message = dbc_mia_replacement_MOTOR_CMD;
    message->mia_info = previous_mia;
  }

  return message_just_entered_mia;
}

static inline bool dbc_service_mia_MOTOR_STATUS(dbc_MOTOR_STATUS_s *message, const uint32_t increment_mia_by) {
  const bool message_just_entered_mia = dbc_service_mia_for(&(message->mia_info), increment_mia_by, dbc_mia_threshold_MOTOR_STATUS);

  if (message_just_entered_mia) {
    const dbc_mia_info_t previous_mia = message->mia_info;
    *message = dbc_mia_replacement_MOTOR_STATUS;
    message->mia_info = previous_mia;
  }

  return message_just_entered_mia;
}

static inline bool dbc_service_mia_SENSOR_SONARS(dbc_SENSOR_SONARS_s *message, const uint32_t increment_mia_by) {
  const bool message_just_entered_mia = dbc_service_mia_for(&(message->mia_info), increment_mia_by, dbc_mia_threshold_SENSOR_SONARS);

  if (message_just_entered_mia) {
    const dbc_mia_info_t previous_mia = message->mia_info;
    *message = dbc_mia_replacement_SENSOR_SONARS;
    message->mia_info = previous_mia;
  }

  return message_just_entered_mia;
}

static inline bool dbc_service_mia_DBC_TEST4(dbc_DBC_TEST4_s *message, const uint32_t increment_mia_by) {
  const bool message_just_entered_mia = dbc_service_mia_for(&(message->mia_info), increment_mia_by, dbc_mia_threshold_DBC_TEST4);

  if (message_just_entered_mia) {
    const dbc_mia_info_t previous_mia = message->mia_info;
    *message = dbc_mia_replacement_DBC_TEST4;
    message->mia_info = previous_mia;
  }

  return message_just_entered_mia;
}

// clang-format off
