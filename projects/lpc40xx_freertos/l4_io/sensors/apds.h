#pragma once

#include <stdbool.h>
#include <stdint.h>

bool apds__init(void);

uint8_t apds__get_proximity(void);
