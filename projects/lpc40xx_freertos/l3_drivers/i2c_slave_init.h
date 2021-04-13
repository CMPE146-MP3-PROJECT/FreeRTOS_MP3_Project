// TODO: Create i2c_slave_init.h
#include "gpio.h"
#include "i2c.h"
#include "lpc40xx.h"
#include <stdbool.h>
#include <stdint.h>

void peripherals_init__i2c1_init(uint8_t slave_address_to_respond_to);
