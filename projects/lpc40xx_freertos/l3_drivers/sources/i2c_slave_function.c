#include "i2c_slave_function.h"

// Private Variables
static volatile uint8_t slave_memory[256];

bool i2c_slave_callback__read_memory(uint8_t memory_index, uint8_t *memory) {
  if (memory_index < 256) {
    slave_memory[0] = 0xA;
    *memory = slave_memory[memory_index];
    return true;
  } else {
    return false;
  }
}

bool i2c_slave_callback__write_memory(uint8_t memory_index, uint8_t memory_value) {
  if (memory_index < 256) {
    slave_memory[memory_index] = memory_value;
    return true;
  } else {
    return false;
  }
}

uint8_t read_memory(uint8_t index) { return slave_memory[index]; }
