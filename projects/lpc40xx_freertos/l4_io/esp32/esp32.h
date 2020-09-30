#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "uart.h"

/// Initializes the ESP32 wifi module's uart port that will be used for future communication
void esp32__init(uart_e esp32_uart_interface);

/// Discards all input from UART until 'timeout_ms' have elapsed without any UART input
void esp32__clear_receive_buffer(uint32_t timeout_ms);

/// Sends command to ESP32 followed by \r\n (which should not be included in the command)
void esp32__send_command(const char *command);

/**
 * Continues to receive responses until we match the 'expected_response'
 * If timeout occurs while receiving data, then this function will return with failure (bool = false)
 */
bool esp32__wait_for_response(const char *expected_response);

/**
 * - Sends the command to the ESP32
 * - Waits until expected response is received
 * - If expected response is not received, then 'error_message' is printed to standard output
 */
bool esp32__wait_for_successful_command(const char *command, const char *expected_response, const char *error_message);

/**
 * Issues wifi connect command with given SSID and password
 */
bool esp32__wifi_connect(const char *ssid, const char *password);

/**
 * Establishes TCP connection with the given host_or_ip on the tcp_server_port
 */
bool esp32__tcp_connect(const char *host_or_ip, uint16_t tcp_server_port);

/**
 * Sends data to an open TCP connection
 */
bool esp32__cipsend(const char *buffer, size_t length);
