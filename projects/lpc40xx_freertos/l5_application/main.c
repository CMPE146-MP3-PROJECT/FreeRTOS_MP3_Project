#include "FreeRTOS.h"
#include "task.h"
//#include <mutex>
#include "board_io.h"
#include "common_macros.h"
#include "lpc40xx.h"
#include "periodic_scheduler.h"
#include "semphr.h"
#include "sj2_cli.h"
#include <stdio.h>

// 'static' to make these functions 'private' to this file
static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);
static SemaphoreHandle_t binary_semaphore;
static SemaphoreHandle_t mutex;
static const uint32_t pin26 = (1 << 26); // 0x02000000? LED1后面写了: P2_3; LED2后面写了: P1_26

// lab 2 part 0
/*void lab2_led_task(void *pvParameters) {
  // Choose one of the onboard LEDS by looking into schematics and write code for the below
  // 0) Set the IOCON MUX function(if required) select pins to 000
  LPC_IOCON->P2_3 &= ~(7 << 0);
  // 0x1000100101 & 0x000 = 0x1000100000
  // 1) Set the DIR register bit for the LED port pin, 1:output, 0:input
  LPC_GPIO2->DIR |= (1 << 3); // 0001 << 3 = 1000; which port to use does not matter
  // 0xFFAB56DF
  // 0x00000008
  //=0xFFAB56DF
  while (true) {
    // 2) Set PIN register bit to 0 to turn ON LED (led may be active low)
    LPC_GPIO2->SET = (1 << 3);
    vTaskDelay(500);

    // 3) Set PIN register bit to 1 to turn OFF LED
    LPC_GPIO2->CLR = (1 << 3);
    vTaskDelay(500);
  }
}*/

// lab 2 part 1
static void lab2_led_task(void *pvParameters) {
  // Set the IOCON MUX function(if required) select pins to 000
  LPC_IOCON->P2_3 &= ~(7 << 0); //~0111 = 1000
  // port_pin_s led3 = {1, 26};
  gpio0__set_as_output(3);
  while (1) {
    // turn the LED on using CLR register
    gpio0__set_high(3);
    vTasDelay(500); // delay for batter result showing

    // turn the LED off using SET register
    gpio0__set_low(3);
    vTaskDelay(500);
  }
}

// lab 2 part 2
int main(void) {
  // lab 2 part 0, 1
  xTaskCreate(lab2_led_task, "LED", 1024 / sizeof(void *), NULL, 1, NULL);
  vTaskStartScheduler();
  return 0;
}

// This sends periodic messages over printf() which uses system_calls.c to send them to UART0

static void uart_task(void *params) {
  TickType_t previous_tick = 0;
  TickType_t ticks = 0;

  while (true) {
    // This loop will repeat at precise task delay, even if the logic below takes variable amount of ticks
    vTaskDelayUntil(&previous_tick, 2000);

    /* Calls to fprintf(stderr, ...) uses polled UART driver, so this entire output will be fully
     * sent out before this function returns. See system_calls.c for actual implementation.
     *
     * Use this style print for:
     *  - Interrupts because you cannot use printf() inside an ISR
     *    This is because regular printf() leads down to xQueueSend() that might block
     *    but you cannot block inside an ISR hence the system might crash
     *  - During debugging in case system crashes before all output of printf() is sent
     */
    ticks = xTaskGetTickCount();
    fprintf(stderr, "%u: This is a polled version of printf used for debugging ... finished in", (unsigned)ticks);
    fprintf(stderr, " %lu ticks\n", (xTaskGetTickCount() - ticks));

    /* This deposits data to an outgoing queue and doesn't block the CPU
     * Data will be sent later, but this function would return earlier
     */
    ticks = xTaskGetTickCount();
    printf("This is a more efficient printf ... finished in");
    printf(" %lu ticks\n\n", (xTaskGetTickCount() - ticks));
  }
}
/*
static void hw2_led_task(void *pvParameters) {
  // Choose one of the onboard LEDS by looking into schematics and write code for the below
  // 1) Set the PIN register bits for the LED//P2_3
  // 2) Set the DIR register bit for the LED
  LPC_IOCON->P2_3 &= ~(7 << 0);
  // LPC_GPIO2->DIR |= (1 << 3);
  port_pin_s led3 = {2, 3};
  gpiox__set_as_output(led3);

  LPC_IOCON->P1_26 &= ~(7 << 0);
  port_pin_s led2 = {1, 26};
  gpiox__set_as_output(led2);
  // LPC_GPIO1->DIR |= (1 << 26);

  while (1) {
    // 3) Use CLR register to turn the LED ON (led may be active low)
    // LPC_GPIO2->CLR = (1 << 3);
    // LPC_GPIO1->SET = (1 << 26);
    // LPC_GPIO1->PIN &= ~(1 << 26);
    gpiox__set_high(led3);
    gpiox__set_low(led2);
    //printf("level low: %d", gpiox__get_level(led2));
    vTaskDelay(500);

    // 4) Use SET register to turn the LED OFF
    // LPC_GPIO2->SET = (1 << 3);
    // LPC_GPIO1->CLR = (1 << 26);
    // LPC_GPIO1->PIN |= (1 << 26);
    gpiox__set_low(led3);
    gpiox__set_high(led2);
    // printf("level high: %d ", gpiox__get_level(led2));
    vTaskDelay(500);
  }
}
*/
