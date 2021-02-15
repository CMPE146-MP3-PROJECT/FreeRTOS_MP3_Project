#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "periodic_scheduler.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "task.h"
#include <stdio.h>

// 'static' to make these functions 'private' to this file
static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);
static SemaphoreHandle_t binary_semaphore;
static SemaphoreHandle_t mutex;
static const uint32_t pin26 = (1 << 26); // 0x02000000? LED1后面写了: P2_3; LED2后面写了: P1_26

/// lab 2 part 0
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

/// lab 2 part 1
/*static void lab2_led_task(void *pvParameters) {
  // Set the IOCON MUX function(if required) select pins to 000
  LPC_IOCON->P2_3 &= ~(7 << 0); //~0111 = 1000
  // port_pin_s led3 = {1, 26};
  gpiox__set_as_output(3);
  while (1) {
    // turn the LED on using CLR register
    gpiox__set_high(3);
    vTaskDelay(500); // delay for batter result showing

    // turn the LED off using SET register
    gpiox__set_low(3);
    vTaskDelay(500);
  }
}*/

/// lab 2 part 2 blink together
/*void lab2_led_task(void *task_parameter) {
  // Type-cast the paramter that was passed from xTaskCreate()
  port_pin_s *led_num = (port_pin_s *)(task_parameter);
  LPC_IOCON->P2_3 &= ~(7 << 0);
  LPC_IOCON->P1_26 &= ~(7 << 0);
  gpiox__set_as_output(*led_num);
  while (true) {
    // do: insert code here to blink an LED
    // Hint: Also use vTaskDelay() to sleep the task
    // turn on
    gpiox__set_high(*led_num);
    vTaskDelay(500);
    // turn off
    gpiox__set_low(*led_num);
    vTaskDelay(500);
  }
}*/
/// lab 2 part 2 blink separate
/*void lab2_led_task0(void *task_parameter) {
  // Type-cast the paramter that was passed from xTaskCreate()
  port_pin_s *led_num = (port_pin_s *)(task_parameter);
  LPC_IOCON->P2_3 &= ~(7 << 0);
  gpiox__set_as_output(*led_num);
  while (true) {
    gpiox__set_high(*led_num);
    vTaskDelay(500);
    // turn off
    gpiox__set_low(*led_num);
    vTaskDelay(500);
  }
}
void lab2_led_task1(void *task_parameter) {
  // Type-cast the paramter that was passed from xTaskCreate()
  port_pin_s *led_num = (port_pin_s *)(task_parameter);
  LPC_IOCON->P1_26 &= ~(7 << 0);
  gpiox__set_as_output(*led_num);
  while (true) {
    vTaskDelay(500);
    gpiox__set_high(*led_num);
    vTaskDelay(500);
    // turn off
    gpiox__set_low(*led_num);
    vTaskDelay(500);
  }
}*/

/// lab 2 part3
static SemaphoreHandle_t switch_press_indication;

void lab2_led_task(void *task_parameter) {
  port_pin_s *led_num = (port_pin_s *)(task_parameter);
  gpiox__set_as_output(*led_num);
  puts("entering the led function");
  while (1) {
    printf("Blinking\n");
    gpiox__set_high(*led_num);
    // Note: There is no vTaskDelay() here, but we use sleep mechanism while waiting for the binary semaphore
    if (xSemaphoreTake(switch_press_indication, 1000)) {
      // DO: Blink the LED
      gpiox__set_low(*led_num);
      vTaskDelay(500);
      // gpiox__set_high(*led_num);
      // vTaskDelay(500);
    } else {
      puts("Timeout: No switch press indication for 1000ms");
    }
  }
}

void switch_task(void *task_parameter) {
  port_pin_s *sw = (port_pin_s *)(task_parameter);
  gpiox__set_as_input(*sw);
  puts("entering the switch function");
  while (1) {
    // DO: If switch pressed, set the binary semaphore
    if (gpiox__get_level(*sw)) {
      xSemaphoreGive(switch_press_indication);
      vTaskDelay(100);
    } else {
      puts("No responds");
    }
    // Task should always sleep otherwise they will use 100% CPU
    // This task sleep also helps avoid spurious semaphore give during switch debeounce
  }
}

int main(void) {
  /// lab 2 part 0, 1
  /*xTaskCreate(lab2_led_task, "LED", 1024 / sizeof(void *), NULL, 1, NULL);
  vTaskStartScheduler();
  //return 0;*/

  /// lab 2 part 2
  // This is static such that these variables will be allocated in RAM and not go out of scope
  /*static port_pin_s led0 = {2, 3};
  static port_pin_s led1 = {1, 26};
  xTaskCreate(lab2_led_task0, "LED0", 1024 / sizeof(void *), &led0, 1, NULL); // &led0 is a task parameter going to
  // vTaskDelay(1500);
  xTaskCreate(lab2_led_task1, "LED1", 1024 / sizeof(void *), &led1, 1, NULL);
  // vTaskDelay(1500);
  vTaskStartScheduler();
  // return 0;*/

  /// lab 2 part 3
  switch_press_indication = xSemaphoreCreateBinary();
  static port_pin_s test_led = {2, 3};     // SW0
  static port_pin_s test_switch = {1, 10}; // LED0
  // printf("level: %d", gpiox__get_level(test_switch))
  xTaskCreate(switch_task, "switch_test", 1024 / sizeof(void *), &test_switch, 1, NULL);
  xTaskCreate(lab2_led_task, "led_test", 1024 / sizeof(void *), &test_led, 1, NULL);
  // return 0;
}
