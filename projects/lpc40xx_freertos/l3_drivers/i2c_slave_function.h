#pragma once

#include "string.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * Use memory_index and read the data to *memory pointer
 * return true if everything is well
 */
bool i2c_slave_callback__read_memory(uint8_t memory_index, uint8_t *memory);

/**
 * Use memory_index to write memory_value
 * return true if this write operation was valid
 */
bool i2c_slave_callback__write_memory(uint8_t memory_index, uint8_t memory_value);

int i2c_memory_utilization(void);
