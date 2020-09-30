#include <stdio.h>
#include <string.h>

#include "esp32.h"

#include "uart_printf.h"

typedef char esp32__buffer[128];
static uart_e esp32_uart = UART__3;

static const size_t esp32__default_retry_count = 3;
static const uint32_t esp32__default_timeout_ms = 3000;

static void esp32__print_response(char *response) {
  // Do not print just an empty response
  const size_t response_strlen = strlen(response);

  if ((response_strlen > 0) && (0 != strcmp(response, "\r\n"))) {
    printf("....ESP32 response: ");

    for (size_t index = 0; index < response_strlen; index++) {
      printf(" %02X", response[index]);
    }

    // Remove trailing newline characters
    if (response_strlen > 2 && response[response_strlen - 1] == '\n') {
      response[response_strlen - 2] = '\0';
    }

    printf("=%s\r\n", response);
  }
}

static bool esp32__receive_response(char *response_buffer, size_t response_buffer_max_size) {
  bool timeout = false;
  memset(response_buffer, 0, response_buffer_max_size);

  for (size_t index = 0; index < response_buffer_max_size; index++) {
    if (!uart__get(esp32_uart, &response_buffer[index], esp32__default_timeout_ms)) {
      timeout = true;
    }

    if (timeout || '\n' == response_buffer[index]) {
      break;
    }
  }

  // Null terminate if overflow
  if (response_buffer_max_size > 0) {
    response_buffer[response_buffer_max_size - 1] = '\0';
  }

  esp32__print_response(response_buffer);

  return timeout;
}

void esp32__init(uart_e esp32_uart_interface) { esp32_uart = esp32_uart_interface; }

void esp32__clear_receive_buffer(uint32_t timeout_ms) {
  char byte = 0;

  while (uart__get(esp32_uart, &byte, timeout_ms)) {
    // Somehow printing excessive chars crashes the 'Telemetry' serial console (sjsu-dev2.github.io)
    // fprintf(stderr, "%c", byte);
  }
}

void esp32__send_command(const char *command) {
  if (NULL != command) {
    printf("  ESP32 transmit: %s\n", command);
    uart_puts(esp32_uart, command);
  }
}

bool esp32__wait_for_response(const char *expected_response) {
  esp32__buffer response = {0};
  bool matched_response = false;

  if (NULL != expected_response) {
    while (!esp32__receive_response(response, sizeof(response))) {
      // Check if expected response is at the beginning; this will ignore the '\r\n' chars of response
      if (response == strstr(response, expected_response)) {
        matched_response = true;
        break;
      }

      if (response == strstr(response, "ERROR")) {
        break;
      }
    }
  }

  return matched_response;
}

bool esp32__wait_for_successful_command(const char *command, const char *expected_response, const char *error_message) {
  bool success = false;

  if (NULL != command && NULL != expected_response && NULL != error_message) {
    for (size_t retries = 0; retries < esp32__default_retry_count; retries++) {
      esp32__send_command(command);

      if (esp32__wait_for_response(expected_response)) {
        success = true;
        break;
      }

      printf(" ERROR: ESP32 not responding to AT command: %s: (%s)\n", command, error_message);
      vTaskDelay(esp32__default_timeout_ms);
    }
  }

  return success;
}

bool esp32__wifi_connect(const char *ssid, const char *password) {
  esp32__buffer wifi_connect_command = {0};
  if (NULL != ssid && NULL != password) {
    snprintf(wifi_connect_command, sizeof(wifi_connect_command), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
  }

  /* Expected response
   * 'WIFI CONNECTED'
   * 'WIFI GOT IP'
   * 'OK'
   */
  return esp32__wait_for_successful_command(wifi_connect_command, "OK", "WIFI connection");
}

bool esp32__tcp_connect(const char *host_or_ip, uint16_t tcp_server_port) {
  // 'AT+CIPSTART="TCP","192.168.1.66",1000'
  esp32__buffer command = {0};

  if (NULL != host_or_ip) {
    snprintf(command, sizeof(command), "AT+CIPSTART=\"TCP\",\"%s\",%u", host_or_ip, (unsigned)tcp_server_port);
  }

  return esp32__wait_for_successful_command(command, "OK", "Connect to TCP as client");
}

static bool esp32__cipsend_packet(const char *buffer, size_t packet_size_in_bytes) {
  bool success = false;

  for (size_t byte_counter = 0; byte_counter < packet_size_in_bytes; byte_counter++) {
    const char byte_to_send = buffer[byte_counter];

    // Some ESP modules indicate that we have to 'escape' the data, but this logic
    // was tested and deemed not necessary for the ESP module used for test
    const char escape_byte = '\\';
    if ('\0' == byte_to_send || escape_byte == byte_to_send) {
      // uart__put(esp32_uart, escape_byte, UINT32_MAX);
    }

    uart__put(esp32_uart, byte_to_send, UINT32_MAX);

    // In case you want to echo data back to standart output
    // putchar(byte_to_send);
  }

  if (esp32__wait_for_response("SEND OK")) {
    success = true;
  }

  return success;
}

bool esp32__cipsend(const char *buffer, size_t length) {
  bool success = true;

  if (NULL != buffer && length > 0U) {
    esp32__buffer command = {0};
    size_t bytes_remaining = length;
    const size_t max_packet_size = 1000; // Prefer sizes less than 1460 bytes (below typical MTU)

    while (bytes_remaining > 0U) {
      const size_t packet_size = (bytes_remaining > max_packet_size) ? max_packet_size : bytes_remaining;

      snprintf(command, sizeof(command), "AT+CIPSEND=%u", packet_size);
      esp32__send_command(command);
      esp32__wait_for_response("OK");

      // ESP32 should send the '>' char which is to indicate that we can now send the data
      char byte = 0;
      if (!uart__get(esp32_uart, &byte, esp32__default_timeout_ms)) {
        printf("ERROR: Expected the > char from ESP32\n");
      }

      const char expected_byte = '>';
      const bool clear_to_send = (expected_byte == byte);

      if (clear_to_send) {
        success = esp32__cipsend_packet(buffer, packet_size);
      }

      buffer += packet_size;
      bytes_remaining -= packet_size;
    }
  }

  return success;
}