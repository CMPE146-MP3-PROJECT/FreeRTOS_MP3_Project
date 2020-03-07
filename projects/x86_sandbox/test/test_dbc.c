#include "unity.h"

#include "dbc.h"

const uint32_t dbc_mia_threshold_DBC_TEST1 = 100;
const uint32_t dbc_mia_threshold_DBC_TEST2 = 100;
const uint32_t dbc_mia_threshold_DBC_TEST3 = 100;
const uint32_t dbc_mia_threshold_DRIVER_HEARTBEAT = 100;
const uint32_t dbc_mia_threshold_MOTOR_CMD = 100;
const uint32_t dbc_mia_threshold_MOTOR_STATUS = 100;
const uint32_t dbc_mia_threshold_SENSOR_SONARS = 100;
const uint32_t dbc_mia_threshold_DBC_TEST4 = 100;

const dbc_DBC_TEST1_s dbc_mia_replacement_DBC_TEST1;
const dbc_DBC_TEST2_s dbc_mia_replacement_DBC_TEST2;
const dbc_DBC_TEST3_s dbc_mia_replacement_DBC_TEST3;
const dbc_DRIVER_HEARTBEAT_s dbc_mia_replacement_DRIVER_HEARTBEAT;
const dbc_MOTOR_CMD_s dbc_mia_replacement_MOTOR_CMD;
const dbc_MOTOR_STATUS_s dbc_mia_replacement_MOTOR_STATUS;
const dbc_SENSOR_SONARS_s dbc_mia_replacement_SENSOR_SONARS;
const dbc_DBC_TEST4_s dbc_mia_replacement_DBC_TEST4;

void test_encode_decode__dbc_DBC_TEST1_s() {
  dbc_DBC_TEST1_s msg = {};
  msg.DBC_TEST1_enum = DBC_TEST1_enum_val_one;
  msg.DBC_TEST1_float = 1.234;
  msg.DBC_TEST1_float_signed = 2.345;
  msg.DBC_TEST1_unsigned1 = 255;
  msg.DBC_TEST1_unsigned_minmax = 34;

  // Basic encoding
  uint8_t bytes[8];
  const dbc_message_header_t header = dbc_encode_DBC_TEST1(bytes, &msg);
  TEST_ASSERT_EQUAL(500, header.message_id);
  TEST_ASSERT_EQUAL(8, header.message_dlc);

  // Decoding with bad header
  dbc_DBC_TEST1_s decoded;
  const dbc_message_header_t bad_header;
  TEST_ASSERT_FALSE(dbc_decode_DBC_TEST1(&decoded, bad_header, bytes));

  // Validate decoded signals
  TEST_ASSERT_TRUE(dbc_decode_DBC_TEST1(&decoded, header, bytes));
  TEST_ASSERT_EQUAL(DBC_TEST1_enum_val_one, decoded.DBC_TEST1_enum);
  TEST_ASSERT_EQUAL(1.234, decoded.DBC_TEST1_float);
  TEST_ASSERT_EQUAL(2.345, decoded.DBC_TEST1_float_signed);
  TEST_ASSERT_EQUAL(255, decoded.DBC_TEST1_unsigned1);
  TEST_ASSERT_EQUAL(34, decoded.DBC_TEST1_unsigned_minmax);
}

void test_encode_decode__dbc_DBC_TEST2_s() {
  dbc_DBC_TEST2_s decoded = {};
  dbc_DBC_TEST2_s msg = {};

  msg.DBC_TEST2_real_signed1 = -100;
  msg.DBC_TEST2_real_signed2 = +100;
  msg.DBC_TEST2_signed = -128;
  msg.DBC_TEST2_signed_minmax = 10000;

  // Basic encoding
  uint8_t bytes[8];
  const dbc_message_header_t header = dbc_encode_DBC_TEST2(bytes, &msg);
  TEST_ASSERT_EQUAL(501, header.message_id);
  TEST_ASSERT_EQUAL(8, header.message_dlc);

  // Validate decoded signals
  TEST_ASSERT_TRUE(dbc_decode_DBC_TEST2(&decoded, header, bytes));
  TEST_ASSERT_EQUAL(-100, decoded.DBC_TEST2_real_signed1);
  TEST_ASSERT_EQUAL(+100, decoded.DBC_TEST2_real_signed2);
  TEST_ASSERT_EQUAL(-128, decoded.DBC_TEST2_signed);
  TEST_ASSERT_EQUAL(10000, decoded.DBC_TEST2_signed_minmax);

  msg.DBC_TEST2_signed_minmax = -10000;
  dbc_encode_DBC_TEST2(bytes, &msg);
  TEST_ASSERT_TRUE(dbc_decode_DBC_TEST2(&decoded, header, bytes));
  TEST_ASSERT_EQUAL(-10000, decoded.DBC_TEST2_signed_minmax);
}

void test_encode_decode__dbc_encode_MOTOR_CMD() {
  dbc_MOTOR_CMD_s decoded = {};
  dbc_MOTOR_CMD_s msg = {};
  uint8_t bytes[8];

  // min values
  msg.MOTOR_CMD_drive = 0;
  msg.MOTOR_CMD_steer = -5;
  const dbc_message_header_t header = dbc_encode_MOTOR_CMD(bytes, &msg);
  TEST_ASSERT_TRUE(dbc_decode_MOTOR_CMD(&decoded, header, bytes));
  TEST_ASSERT_EQUAL(0, msg.MOTOR_CMD_drive);
  TEST_ASSERT_EQUAL(-5, msg.MOTOR_CMD_steer);

  // max values
  msg.MOTOR_CMD_drive = 9;
  msg.MOTOR_CMD_steer = +5;
  dbc_encode_MOTOR_CMD(bytes, &msg);
  TEST_ASSERT_TRUE(dbc_decode_MOTOR_CMD(&decoded, header, bytes));
  TEST_ASSERT_EQUAL(9, decoded.MOTOR_CMD_drive);
  TEST_ASSERT_EQUAL(+5, decoded.MOTOR_CMD_steer);

  // out of bounds on negative
  msg.MOTOR_CMD_drive = 0;
  msg.MOTOR_CMD_steer = -60;
  dbc_encode_MOTOR_CMD(bytes, &msg);
  TEST_ASSERT_TRUE(dbc_decode_MOTOR_CMD(&decoded, header, bytes));
  TEST_ASSERT_EQUAL(-0, decoded.MOTOR_CMD_drive);
  TEST_ASSERT_EQUAL(-5, decoded.MOTOR_CMD_steer);

  // out of bounds on positive
  msg.MOTOR_CMD_drive = 100;
  msg.MOTOR_CMD_steer = 6;
  dbc_encode_MOTOR_CMD(bytes, &msg);
  TEST_ASSERT_TRUE(dbc_decode_MOTOR_CMD(&decoded, header, bytes));
  TEST_ASSERT_EQUAL(9, decoded.MOTOR_CMD_drive);
  TEST_ASSERT_EQUAL(5, decoded.MOTOR_CMD_steer);
}

void test_mia_counter(void) {
  dbc_MOTOR_STATUS_s msg = {};
  TEST_ASSERT_FALSE(dbc_service_mia_MOTOR_STATUS(&msg, 10));
  TEST_ASSERT_FALSE(dbc_service_mia_MOTOR_STATUS(&msg, 40));
  TEST_ASSERT_FALSE(dbc_service_mia_MOTOR_STATUS(&msg, 40));

  TEST_ASSERT_TRUE(dbc_service_mia_MOTOR_STATUS(&msg, 10));

  // Should remain MIA
  TEST_ASSERT_FALSE(dbc_service_mia_MOTOR_STATUS(&msg, 10));
  TEST_ASSERT_FALSE(dbc_service_mia_MOTOR_STATUS(&msg, 10));
}

void test_mia_replacement(void) {
  dbc_MOTOR_STATUS_s msg = {};
  msg.MOTOR_STATUS_speed_kph = 10;
  TEST_ASSERT_FALSE(dbc_service_mia_MOTOR_STATUS(&msg, 90));

  TEST_ASSERT_EQUAL(10, msg.MOTOR_STATUS_speed_kph);
  TEST_ASSERT_TRUE(dbc_service_mia_MOTOR_STATUS(&msg, 10));
  TEST_ASSERT_EQUAL(0, msg.MOTOR_STATUS_speed_kph);

  // already MIA so should not be changed
  msg.MOTOR_STATUS_speed_kph = 20;
  TEST_ASSERT_FALSE(dbc_service_mia_MOTOR_STATUS(&msg, 10));
  TEST_ASSERT_EQUAL(20, msg.MOTOR_STATUS_speed_kph);
}

bool dbc_send_can_message(void *argument, uint32_t message_id, const uint8_t bytes[8], uint8_t dlc) {
  TEST_ASSERT_EQUAL(dbc_header_MOTOR_STATUS.message_id, message_id);
  TEST_ASSERT_EQUAL(dbc_header_MOTOR_STATUS.message_dlc, dlc);
  TEST_ASSERT_EQUAL((void *)0xDEADBEEF, argument);
  return true;
}

void test_encode_and_send(void) {
  dbc_MOTOR_STATUS_s msg = {};
  TEST_ASSERT_TRUE(dbc_encode_and_send_MOTOR_STATUS((void *)0xDEADBEEF, &msg));
}