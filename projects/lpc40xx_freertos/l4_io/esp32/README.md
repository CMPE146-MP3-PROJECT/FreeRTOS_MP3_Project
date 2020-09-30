ESP32 or ESP8266 are wifi modules that can be installed on the SJ2 board. The interface to these modules is through UART3, and the module allows us to interact with TCP or UDP sockets using "AT" commands that the ESP module supports.

## Task

The following code can be used at your `main()` to run a "hello world" program for the ESP module that will attempt to connect to a TCP/IP server and send some data.

```c
#include "esp32_task.h"
#include "uart3_init.h"

int main(void) {
  uart3_init(); // Also include:  uart3_init.h
  xTaskCreate(esp32_tcp_hello_world_task, "uart3", 1000, NULL, PRIORITY_LOW, NULL); // Include esp32_task.h

  // ...
  vTaskStartScheduler();
  return 0;
}
```

## SD Card for Wifi Parameters

The wifi key, wifi password, TCP server IP, and port are parameters that must be stored in the micro-SD card such that the runtime system can read this data to connect to TCP server. This provides flexibility such that the program on the SJ2 does not need to be re-compiled in order to change these parameters.

The `esp_task.c` has a task that will attempt to do the following to read your parameters to run a hello world test program.

1. Save `wifi.txt` to the micro-SD card to read wifi key and password

This file should have two lines with wifi key and password; example:
```
wifi_key
wifi_password
```

2. Save `tcp_server.txt` to the micro-SD card to store target TCP server to connect to.

This file should have two lines with TCP host(or IP) and port number; example:
```
192.168.1.10
2020
```

## Test

Once the code integration is complete, along with the files that you should have saved on the SD card, you are ready to run your test.

On a linux or mac computer (god help you for windows), run the `nc -l 2020` or `netcat -l 2020` and this will "host" the TCP server and allow the ESP32 to send the data. Note that the port number of `2020` should match the data saved at `tcp_server.txt`.

If all goes well, you should get the following output in your serial console. You could comment out the line that prints out the `ESP32 response` to reduce the "noise".

```
  ESP32 transmit: AT+RST
====================================
Starting communication with ESP32
====================================
  ESP32 transmit: AT
....ESP32 response:  41 54 0D 0D 0A=AT
....ESP32 response:  4F 4B 0D 0A=OK
  ESP32 transmit: ATE0
....ESP32 response:  41 54 45 30 0D 0D 0A=ATE0
....ESP32 response:  4F 4B 0D 0A=OK
  ESP32 transmit: AT+CIPMUX=0
....ESP32 response:  4F 4B 0D 0A=OK
  ESP32 transmit: AT+CWMODE=1
....ESP32 response:  4F 4B 0D 0A=OK
====================================
ESP32 initialized
====================================
Using ssid/password from file wifi.txt to connect to wifi
  ESP32 transmit: AT+CWJAP="ssid","password"
....ESP32 response:  57 49 46 49 20 44 49 53 43 4F 4E 4E 45 43 54 0D 0A=WIFI DISCONNECT
....ESP32 response:  57 49 46 49 20 43 4F 4E 4E 45 43 54 45 44 0D 0A=WIFI CONNECTED
....ESP32 response:  57 49 46 49 20 47 4F 54 20 49 50 0D 0A=WIFI GOT IP
....ESP32 response:  4F 4B 0D 0A=OK
TCP server information: 192.168.1.66:1000
  ESP32 transmit: AT+CIPSTART="TCP","192.168.1.66",1000
....ESP32 response:  43 4F 4E 4E 45 43 54 0D 0A=CONNECT
....ESP32 response:  4F 4B 0D 0A=OK
  ESP32 transmit: AT+CIPSEND=8
....ESP32 response:  4F 4B 0D 0A=OK
....ESP32 response:  20 0D 0A=
....ESP32 response:  52 65 63 76 20 38 20 62 79 74 65 73 0D 0A=Recv 8 bytes
....ESP32 response:  53 45 4E 44 20 4F 4B 0D 0A=SEND OK
```