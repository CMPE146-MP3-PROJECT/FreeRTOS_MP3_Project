#include "i2c_slave_function.h"

// Private Variables
static volatile uint8_t slave_memory[256] = {0};

bool i2c_slave_callback__read_memory(uint8_t memory_index, uint8_t *memory) {
  if (memory_index < 256) {
    slave_memory[100] = 0xA; // manually put a data into the array just for testing
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

int i2c_memory_utilization(void) {
  // int size = sizeof(slave_memory) / sizeof(slave_memory[0]);
  int size;
  for (int i = 0; i < 255; i++) {
    if (slave_memory[i] != 0) {
      size++;
    }
  }
  return size;
}
