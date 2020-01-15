/**
 * @file
 * Analog to Digital Converter driver for LPC40xx
 *
 * @note
 * This driver is intentionally simplified, and is meant to be used as a reference.
 * Burst mode should be enabled for a better implementation
 */
#pragma once

#include <stdint.h>

// Only Channel2, Channel4 and Channel5 pins are avaible for use on SJ2 Development board
typedef enum {
  ADC__CHANNEL_2 = 2, // Pin Available on SJ2
  ADC__CHANNEL_4 = 4, // Pin Available on SJ2
  ADC__CHANNEL_5 = 5, // Pin Available on SJ2
} adc_channel_e;

void adc__initialize(void);

/**
 * Reads the given ADC channal and returns its digital value
 * This starts conversion of one channel, and should not be used from multiple tasks
 */
uint16_t adc__get_adc_value(adc_channel_e channel_num);
