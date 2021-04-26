#pragma once
#include "DAC_functions.h"
#include "FreeRTOS.h"
#include "OLED.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "queue.h"
#include "semphr.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern QueueHandle_t Q_songname;
typedef char songname_backup[32];

// key_functions_init();

key_functions(char key_detected); // don't know if the char has to be a * ???