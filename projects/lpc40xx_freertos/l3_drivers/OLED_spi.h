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
//#include "mp3_functions.h"

#include "task.h"

void spi1__OLED_init(uint32_t max_spi_clock_mhz);

uint8_t spi1__OLED_exchange_byte(uint8_t data_out);

void OLED_cs(void);
void OLED_ds(void);
void OLED_dc(bool status);

void OELD_transfer_data_byte(uint8_t data);

void OELD_transfer_command_byte(uint8_t command);
void OLED_Send_Char(uint8_t data);

void OLED_print_string(uint8_t page, uint8_t upper_col, uint8_t lower_col, uint8_t *data, int length);

void OLED_function_init(void);

void OLED_page_clear(void);

void OLED_Horizontal_Scroll(uint8_t start_page, uint8_t end_page, uint8_t frame_frequency, uint8_t Right_Left,
                            bool auto_function);