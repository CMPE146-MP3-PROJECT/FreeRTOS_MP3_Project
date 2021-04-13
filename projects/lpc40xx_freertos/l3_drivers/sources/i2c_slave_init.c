#include "i2c_slave_init.h"
#include "gpio_lab.h"
/*
/   P0_0 Func3: I2C1_SDA  P0_1 Func3: I2C1_SCL
/   P0_10 Func2: I2C2_SDA  P0_11 Func2: I2C2_SCL
/
*/
void peripherals_init__i2c1_init(uint8_t slave_address_to_respond_to) {

  // uint8_t i2c_interface_enable_mask = (1 << 6); // | (1 << 2);
  // uint8_t slave_address_mask = 0x00;
  // uint8_t assert_ack_flag = (1 << 2);
  uint16_t IOPIN_open_drain_mask = (1 << 10);
  uint8_t slave_address_mask = 0x00;
  i2c__initialize_slave(I2C__0, slave_address_to_respond_to, slave_address_mask);
  // uint8_t slave_address_after_mask = (slave_address_to_respond_to & slave_address_mask);

  // LPC_I2C0->CONSET |= i2c_interface_enable_mask; // enable i2c1 interface
  // LPC_I2C0->CONSET |= assert_ack_flag;           // SET AA bits
  // LPC_I2C0->ADR0 = slave_address_to_respond_to;  // configure slave address
  // LPC_I2C0->MASK0 = slave_address_mask;

  gpio__construct_with_function(GPIO__PORT_0, 10, GPIO__FUNCTION_2); // P0_10 Func2: I2C2_SDA
  // LPC_IOCON->P0_10 |= open_drain_mask;
  gpio__construct_with_function(GPIO__PORT_0, 11, GPIO__FUNCTION_2); // P0_11 Func2: I2C2_SCL
  // LPC_IOCON->P0_11 |= open_drain_mask;

  LPC_IOCON->P1_30 &= ~(7 << 0);
  LPC_IOCON->P1_31 &= ~(7 << 0);
  gpio__construct_with_function(GPIO__PORT_1, 30, GPIO__FUNCTION_4); // I2C0 SDA
  LPC_IOCON->P1_30 |= IOPIN_open_drain_mask;
  gpio__construct_with_function(GPIO__PORT_1, 31, GPIO__FUNCTION_4); // I2C0 SCL
  LPC_IOCON->P1_31 |= IOPIN_open_drain_mask;

  printf("start to find i2c slaves\n");
  for (unsigned slave_address = 2; slave_address <= 254; slave_address += 2) {
    if (i2c__detect(I2C__2, slave_address)) { // scan the i2c peripheral and looking for avaliable slave device
      printf("I2C slave detected at address: 0x%02X\n", slave_address);
    }
  }
}
