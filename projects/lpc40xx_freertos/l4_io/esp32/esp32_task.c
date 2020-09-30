#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp32_task.h"

#include "esp32.h"
#include "ff.h"

static const char *esp32__wifi_filename = "wifi.txt";
static const char *esp32__server_filename = "tcp_server.txt";
static const uart_e esp32__uart = UART__3;

static void trim_newline_chars(char *line) {
  char *trim = NULL;

  trim = strchr(line, '\r');
  if (NULL != trim) {
    *trim = '\0';
  }

  trim = strchr(line, '\n');
  if (NULL != trim) {
    *trim = '\0';
  }
}

static bool read_lines_from_file(const char *filename, char *lines[], size_t line_count, size_t max_line_length) {
  FIL file;
  FRESULT fr;
  bool file_opened = false;

  fr = f_open(&file, filename, FA_READ);
  if (FR_OK == fr) {
    file_opened = true;
    for (size_t line = 0; line < line_count; line++) {
      char *file_line = lines[line];
      f_gets(file_line, max_line_length, &file);
      trim_newline_chars(file_line);
    }
    f_close(&file);
  }

  return file_opened;
}

void esp32_task__init(void) {
  // Reset ESP32 and wait for it to fully power-on
  esp32__init(esp32__uart);
  esp32__send_command("AT+RST");
  esp32__clear_receive_buffer(5000);

  const char *line = "====================================";
  puts(line);
  puts("Starting communication with ESP32");
  puts(line);

  // Test AT command
  esp32__wait_for_successful_command("AT", "OK", "Basic communication test");
  esp32__wait_for_successful_command("ATE0", "OK", "Turn off echo");
  esp32__wait_for_successful_command("AT+CIPMUX=0", "OK", "Single connection");

  // Enter "Station Mode"
  esp32__wait_for_successful_command("AT+CWMODE=1", "OK", "Connect to WIFI");

  puts(line);
  puts("ESP32 initialized");
  puts(line);
}

bool esp32_task__connect_wifi_by_reading_ssid_from_file(void) {
  bool success = false;

  esp32_task__line_buffer_t ssid = {0};
  esp32_task__line_buffer_t password = {0};
  char *lines[] = {ssid, password};

  if (read_lines_from_file(esp32__wifi_filename, lines, 2, sizeof(esp32_task__line_buffer_t))) {
    printf("Using %s/%s from file %s to connect to wifi\n", ssid, password, esp32__wifi_filename);
    esp32__wifi_connect(ssid, password);
    success = true;
  } else {
    printf("ERROR: Unable to read %s to read WIFI SSID and Key\n", esp32__wifi_filename);
  }

  return success;
}

bool esp32_task__get_tcp_server_information(esp32_task__line_buffer_t ip_or_hostname, uint16_t *port) {
  bool success = false;

  esp32_task__line_buffer_t port_string = {0};
  char *lines[] = {ip_or_hostname, port_string};

  if (read_lines_from_file(esp32__server_filename, lines, 2, sizeof(esp32_task__line_buffer_t))) {
    printf("TCP server information: %s:%s\n", ip_or_hostname, port_string);
    *port = (uint16_t)atoi(port_string);
    success = true;
  } else {
    printf("ERROR: Unable to read TCP server information from %s\n", esp32__server_filename);
  }

  return success;
}

/**
 * http://wiki.ai-thinker.com/_media/esp8266/esp8266_series_modules_user_manual_v1.1.pdf
 * https://www.electrokit.com/uploads/productfile/41015/esp32_at_instruction_set_and_examples_en.pdf
 */
void esp32_tcp_hello_world_task(void *params) {
  // Wait for CLI task to output all data
  vTaskDelay(100);

  // Init and connect to wifi
  esp32_task__init();
  if (!esp32_task__connect_wifi_by_reading_ssid_from_file()) {
    vTaskSuspend(NULL);
  }

  // Get TCP/IP information
  esp32_task__line_buffer_t ip_or_hostname = {0};
  uint16_t port = 0;
  if (!esp32_task__get_tcp_server_information(ip_or_hostname, &port)) {
    vTaskSuspend(NULL);
  }

// Sample code if you purely interact over UART3 CLI command and wish to see response of ESP module
#if 0
  while (true) {
    char byte = 0;
    uart__get(esp32_uart, &byte, portMAX_DELAY);
    putchar(byte);
  }
#endif

// Sample code that will connect to TCP/IP server once, and continuously send data
#if 1
  esp32__tcp_connect(ip_or_hostname, port);
  unsigned counter = 0;
  while (1) {
    ++counter;
    char buffer[64] = {0};
    snprintf(buffer, sizeof(buffer), "Test %u\r\n", counter);
    esp32__cipsend(buffer, strlen(buffer));

    vTaskDelay(100);
  }
#endif

// Sample code that will connect, send data, and then close the socket
#if 1
  while (1) {
    esp32__tcp_connect(ip_or_hostname, port);

    const char message_1[] = "hello world\r\n";
    const char message_2[] = "Test Escaped \\ char\r\n";
    const char message_3[] = "Test Escaped \0 NULL char\r\n";
    const char message_4[] = "Done";

    esp32__cipsend(message_1, sizeof(message_1) - 1);
    esp32__cipsend(message_2, sizeof(message_2) - 1);
    esp32__cipsend(message_3, sizeof(message_3) - 1);
    esp32__cipsend(message_4, sizeof(message_4) - 1);

    vTaskDelay(100); // Wait for last cipsend() data to be fully sent out
    esp32__wait_for_successful_command("AT+CIPCLOSE", "OK", "Close connection");
  }
#endif
}