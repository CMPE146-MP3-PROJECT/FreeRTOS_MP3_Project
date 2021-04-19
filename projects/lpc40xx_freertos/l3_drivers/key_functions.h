#pragma once
#include "DAC_functions.h"
#include "FreeRTOS.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "queue.h"
#include "semphr.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

QueueHandle_t Q_key_to_OLED;
SemaphoreHandle_t next_song;

key_functions_init();

key_functions(char key_detected); // don't know if the char has to be a * ???