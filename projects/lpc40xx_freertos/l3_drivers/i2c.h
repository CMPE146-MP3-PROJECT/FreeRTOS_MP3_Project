#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * @file
 *
 * Most typical implementation of an I2C device is that after the device address is sent, the first byte
 * is interpreted as a register number we wish to read or write. Devices that deviate from this
 * implementation may require enhancing this I2C driver.
 *
 * Some devices indicate an 8-bit address, so this an be directly passed to the API as 'slave_address'
 * In this case, we include the read/write bit with the address.
 *
 * Others indicate 7-bit address, and so you should then pass 'addr_7_bit << 1' to 'slave_address' because
 * the LSB bit is a read/write bit.
 */

typedef enum {
  I2C__0,
  I2C__1,
  I2C__2,
} i2c_e;

/**
 * I2C peripheral must be initialized before it is used
 * @param peripheral_clock_hz
 * This is speed of the peripheral clock feeding the I2C peripheral; it is used to set the desired_i2c_bus_speed_in_hz
 */
void i2c__initialize(i2c_e i2c_number, uint32_t desired_i2c_bus_speed_in_hz, uint32_t peripheral_clock_hz);

/**
 * @returns true if the I2C device at the given address responds back with an ACK
 * @note this does not transfer any data
 */
bool i2c__detect(i2c_e i2c_number, uint8_t slave_address);

/**
 * Reads one memory location of a slave_address
 * @param slave_memory_address
 * This is the first byte sent on I2C after the slave_address which typically acts like a slave memory address
 *
 * @note This is the same as i2c__read_slave_data() with number_of_bytes set to 1
 */
uint8_t i2c__read_single(i2c_e i2c_number, uint8_t slave_address, uint8_t slave_memory_address);

/**
 * Reads multiple registers of the slave_address, starting with the starting_slave_memory_address
 * Typical devices implement 'auto increment' of the starting_slave_memory_address which this function relies upon
 */
bool i2c__read_slave_data(i2c_e i2c_number, uint8_t slave_address, uint8_t starting_slave_memory_address,
                          uint8_t *bytes_to_read, uint32_t number_of_bytes);

/**
 * Writes one memory location of a slave_address
 * @param slave_memory_address
 * This is the first byte sent on I2C after the slave_address which typically acts like a slave memory address
 *
 * @note This is the same as i2c__write_slave_data() with number_of_bytes set to 1
 */
bool i2c__write_single(i2c_e i2c_number, uint8_t slave_address, uint8_t slave_memory_address, uint8_t value);

/**
 * Writes multiple registers of the slave_address, starting with the starting_slave_memory_address
 * Typical devices implement 'auto increment' of the starting_slave_memory_address which this function relies upon
 */
bool i2c__write_slave_data(i2c_e i2c_number, uint8_t slave_address, uint8_t starting_slave_memory_address,
                           const uint8_t *bytes_to_write, uint32_t number_of_bytes);
