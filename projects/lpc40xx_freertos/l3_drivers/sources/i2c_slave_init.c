#include "i2c_slave_init.h"
#include "gpio_lab.h"
/*
/   P0_0 Func3: I2C1_SDA  P0_1 Func3: I2C1_SCL
/   P0_10 Func2: I2C2_SDA  P0_11 Func2: I2C2_SCL
/
*/
void peripherals_init__i2c1_init(uint8_t slave_address_to_respond_to) {

  uint8_t i2c_interface_enable_mask = (1 << 6); // | (1 << 2);
  uint8_t slave_address_mask = 0x00;
  uint8_t assert_ack_flag = (1 << 2);
  uint16_t open_drain_mask = (1 << 10);
  const uint32_t i2c_speed_hz = UINT32_C(400) * 1000;
  i2c__initialize(I2C__0, i2c_speed_hz, clock__get_peripheral_clock_hz());
  // uint8_t slave_address_after_mask = (slave_address_to_respond_to & slave_address_mask);

  LPC_I2C0->CONSET |= i2c_interface_enable_mask; // enable i2c1 interface
  LPC_I2C0->CONSET |= assert_ack_flag;           // SET AA bits
  LPC_I2C0->ADR0 = slave_address_to_respond_to;  // configure slave address
  LPC_I2C0->MASK0 = slave_address_mask;
  // port_pin_s SDA1 = {0, 0};
  // port_pin_s SCL1 = {0, 1};
  // port_pin_s SDA2 = {0, 10};
  // port_pin_s SCL2 = {0, 11};
  // gpiox__set_as_input(SDA1);
  // gpiox__set_as_input(SCL1);
  // gpiox__set_as_output(SDA2);
  // gpiox__set_as_output(SCL2);

  // gpio__construct_with_function(GPIO__PORT_0, 0, GPIO__FUNCTION_3); // P0_0 Func3: I2C1_SDA
  // LPC_IOCON->P0_0 |= open_drain_mask;
  // gpio__construct_with_function(GPIO__PORT_0, 1, GPIO__FUNCTION_3); // P0_1 Func3: I2C1_SCL
  // LPC_IOCON->P0_1 |= open_drain_mask;

  gpio__construct_with_function(GPIO__PORT_0, 10, GPIO__FUNCTION_2); // P0_10 Func2: I2C2_SDA
  // LPC_IOCON->P0_10 |= open_drain_mask;
  gpio__construct_with_function(GPIO__PORT_0, 11, GPIO__FUNCTION_2); // P0_11 Func2: I2C2_SCL
  // LPC_IOCON->P0_11 |= open_drain_mask;

  LPC_IOCON->P1_30 &= ~(7 << 0);
  LPC_IOCON->P1_31 &= ~(7 << 0);
  gpio__construct_with_function(GPIO__PORT_1, 30, GPIO__FUNCTION_4); // I2C0 SDA
  LPC_IOCON->P1_30 |= open_drain_mask;
  gpio__construct_with_function(GPIO__PORT_1, 31, GPIO__FUNCTION_4); // I2C0 SCL
  LPC_IOCON->P1_31 |= open_drain_mask;

  // printf("start to find i2c slaves\n");
  // for (unsigned slave_address = 2; slave_address <= 254; slave_address += 2) {
  //   if (i2c__detect(I2C__0, slave_address)) { // scan the i2c2 peripheral and looking for avaliable slave device
  //     printf("I2C slave detected at address: 0x%02X\n", slave_address);
  //   }
  // }
}

// void i2c1__slave_init(uint8_t slave_address_to_respond_to) {
//   uint8_t i2c_interface_enable_mask = (1 << 6); // | (1 << 2);
//   uint8_t slave_address_mask = 0xC0;
//   // uint8_t slave_address_after_mask = (slave_address_to_respond_to & slave_address_mask);
//   LPC_I2C1->CONSET |= i2c_interface_enable_mask; // enable i2c0 interface
//   LPC_I2C1->ADR0 = slave_address_to_respond_to;
//   LPC_I2C1->MASK0 = slave_address_mask;

//   gpio__construct_with_function(GPIO__PORT_0, 0, GPIO__FUNCTION_3); // P0_0 Func3: I2C1_SDA
//   gpio__construct_with_function(GPIO__PORT_0, 1, GPIO__FUNCTION_3); // P0_1 Func3: I2C1_SCL

//   gpio__construct_with_function(GPIO__PORT_0, 10, GPIO__FUNCTION_2); // P0_10 Func2: I2C2_SDA
//   gpio__construct_with_function(GPIO__PORT_0, 11, GPIO__FUNCTION_2); // P0_11 Func2: I2C2_SCL
// }