#pragma once

#include "uart.h"

typedef char esp32_task__line_buffer_t[128];

/**
 * This assumes RTOS is running as it uses task sleep
 */
void esp32_task__init(void);

/**
 * Connects the ESP wifi module by reading wifi key and password from file system
 */
bool esp32_task__connect_wifi_by_reading_ssid_from_file(void);

/**
 * Reads file system to retrieve IP and port that the user needs to connect on
 */
bool esp32_task__get_tcp_server_information(esp32_task__line_buffer_t ip_or_hostname, uint16_t *port);

/**
 * RTOS task that can be created to test TCP connection
 */
void esp32_tcp_hello_world_task(void *params);