#pragma once
#include "FreeRTOS.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "key_functions.h"
#include "queue.h"
#include "semphr.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

SemaphoreHandle_t key_press_indication;
QueueHandle_t Q_keypad_bottom;

void key_pins_init(void);

void read_keys(void);

void IO_pins_value_write(int write_state);

// int *IO_pins_value_read(port_pin_s pin1, port_pin_s pin2, port_pin_s pin3, port_pin_s pin4);
int *IO_pins_value_read(int *result_arrary);