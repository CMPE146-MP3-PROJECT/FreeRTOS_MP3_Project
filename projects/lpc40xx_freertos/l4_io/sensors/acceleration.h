#pragma once

#include <stdbool.h>
#include <stdint.h>

// Data for x-axis, y-axis, and z-axis
typedef struct {
  int16_t x, y, z;
} acceleration__axis_data_s;

/// Enables acceleration sensor data update and returns true if we successfully detected the sensor
bool acceleration__init(void);

/// @returns data of all 3-axis of the sensor
acceleration__axis_data_s acceleration__get_data(void);
