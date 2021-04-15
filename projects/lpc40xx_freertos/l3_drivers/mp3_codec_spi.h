#pragma once

#include "clock.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "task.h"

void adafruit_cs(void);

void adafruit_ds(void);

void spi0__mp3_init(uint32_t max_spi_clock_mhz);

uint8_t spi0__mp3_exchange_byte(uint8_t data_out);
