#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "gpio_isr.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "semphr.h"
#include <stdint.h>
#include <stdio.h>
//#include "lpc_peripherals.h"
#include "acceleration.h"
#include "adc.h"
#include "event_groups.h"
#include "ff.h"
#include "gpio.h"
#include "i2c_slave_init.h"
#include "periodic_scheduler.h"
#include "pwm1.h"
#include "queue.h"
#include "sj2_cli.h"
#include "ssp2.h"
#include "task.h"
#include "uart.h"
#include "uart_lab.h"
/// 'static' to make these functions 'private' to this file
// static void create_blinky_tasks(void);
// static void create_uart_task(void);
// static void blink_task(void *params);
// static void uart_task(void *params);
// static SemaphoreHandle_t binary_semaphore;
// static SemaphoreHandle_t mutex;
// static const uint32_t pin26 = (1 << 26); // 0x02000000? LED1后面写了: P2_3; LED2后面写了: P1_26
static SemaphoreHandle_t switch_press_indication;
static SemaphoreHandle_t switch_pressed_signal;

/// lab 2 part 0
#if 0
void lab2_led_task(void *pvParameters) {
  // Choose one of the onboard LEDS by looking into schematics and write code for the below
  // 0) Set the IOCON MUX function(if required) select pins to 000
  LPC_IOCON->P2_3 &= (7 << 0);
  // 0x1000100101 & 0x000 = 0x1000100000
  // 1) Set the DIR register bit for the LED port pin, 1:output, 0:input
  LPC_GPIO2->DIR |= (1 << 4); // 0001 << 4 = 10000;
  // 0001 << 3 = 1000; which port to use does not matter
  while (true) {
    // 2) Set PIN register bit to 0 to turn ON LED (led may be active low)
    LPC_GPIO2->SET = (1 << 3);
    vTaskDelay(500);

    // 3) Set PIN register bit to 1 to turn OFF LED
    LPC_GPIO2->CLR = (1 << 3);
    vTaskDelay(500);
  }
}
#endif

/// lab 2 part 1
#if 0
static void lab2_led_task(void *pvParameters) {
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
}
#endif

/// lab 2 part 2 blink together
#if 0
void lab2_led_task(void *task_parameter) {
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
}
#endif

/// lab 2 part 2 blink separate
#if 1
void lab2_led_task0(void *task_parameter) {
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
}
#endif

/// lab 2 part3
#if 0
void lab2_led_task(void *task_parameter) {
  // LPC_IOCON->P2_3 &= ~(7 << 0);
  const port_pin_s *led_num = (port_pin_s *)(task_parameter);
  gpiox__set_as_output(*led_num);
  while (true) {
    printf("entering the led function\n");
    // gpiox__set_high(*led_num);
    // Note: There is no vTaskDelay() here, but we use sleep mechanism while waiting for the binary semaphore
    if (xSemaphoreTake(switch_press_indication, 1000)) {
      // Blink the LED
      printf("Blinking\n");
      gpiox__set_high(*led_num);
      vTaskDelay(500);
      gpiox__set_low(*led_num);
      vTaskDelay(500);
    } else {
      puts("Timeout: No switch press indication for 1000ms");
    }
  }
}

void switch_task(void *task_parameter) {
  port_pin_s *switch0 = (port_pin_s *)(task_parameter);
  // gpiox__set_as_input(*switch0);
  // printf("level: %d", gpiox__get_level(*switch0));
  while (true) {
    // DO: If switch pressed, set the binary semaphore
    if (gpiox__get_level(*switch0)) {
      xSemaphoreGive(switch_press_indication);
      vTaskDelay(100);
    }
    // Task should always sleep otherwise they will use 100% CPU
    // This task sleep also helps avoid spurious semaphore give during switch debeounce
  }
}
#endif

/// lab 3 part 0
#if 0
void gpio_interrupt(void) {
  // a) Clear Port0/2 interrupt using CLR0 or CLR2 registers
  LPC_GPIOINT->IO0IntClr = (1 << 29);    // clr the INT register for the switch's Port/Pin
                                         // (the switch that trigger interrupt)
  static port_pin_s test_led2 = {1, 24}; // LED2
  static port_pin_s test_led1 = {1, 26}; // LED1

  // b) Use fprintf(stderr) or blink and LED here to test your ISR
  LPC_IOCON->P1_26 &= ~(7 << 0);
  gpiox__set_as_output(test_led1);
  gpiox__set_high(test_led1);
  delay__ms(150);
  gpiox__set_low(test_led1);
  delay__ms(150);
  fprintf(stderr, "HEY THERE!\n");
}
#endif

/// lab 3 part 1
#if 1
void gpio_interrupt2(void) {
  fprintf(stderr, "Calling ISR...");
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
  LPC_GPIOINT->IO0IntClr = (1 << 30); // clr the INT register for the switch's Port/Pin
                                      // (the switch that trigger interrupt)
}
void sleep_on_sem_task(void *p) {
  const port_pin_s *sem_led = (port_pin_s *)(p);
  gpiox__set_as_output(*sem_led);
  while (1) {
    if (xSemaphoreTake(switch_pressed_signal, 1000000)) {
      fprintf(stderr, "Servicing Interrupt\n");
      gpiox__set_high(*sem_led);
      delay__ms(150);
      gpiox__set_low(*sem_led);
      delay__ms(150); // blinking LED
      // vTaskDelay(100);
    }
    // Use xSemaphoreTake with forever delay and blink an LED when you get the signal
  }
}
#endif

/// lab 3 part 2
#if 0
static SemaphoreHandle_t switch_pressed_signal_hw3_part2_30;
static SemaphoreHandle_t switch_pressed_signal_hw3_part2_31;
void gpio_interrupt_part2_0(void) { xSemaphoreGiveFromISR(switch_pressed_signal_hw3_part2_30, NULL); } // ISR 1
void gpio_interrupt_part2_1(void) { xSemaphoreGiveFromISR(switch_pressed_signal_hw3_part2_31, NULL); } // ISR 2

void pin30_isr(void *p) {
  const port_pin_s *sem_led = (port_pin_s *)(p);
  gpiox__set_as_output(*sem_led);
  while (1) {
    if (xSemaphoreTake(switch_pressed_signal_hw3_part2_30, 1000000)) {
      fprintf(stderr, "Servicing Interrupt 1\n");
      gpiox__set_high(*sem_led);
      delay__ms(100);
      gpiox__set_low(*sem_led);
      delay__ms(100); // blinking LED
      gpiox__set_high(*sem_led);
      delay__ms(100);
      gpiox__set_low(*sem_led);
      delay__ms(100); // blinking LED
      // vTaskDelay(100);
    }
    // Use xSemaphoreTake with forever delay and blink an LED when you get the signal
  }
}
void pin31_isr(void *p) {
  const port_pin_s *sem_led = (port_pin_s *)(p);
  gpiox__set_as_output(*sem_led);
  while (1) {
    if (xSemaphoreTake(switch_pressed_signal_hw3_part2_31, 1000000)) {
      fprintf(stderr, "Servicing Interrupt 2\n");
      gpiox__set_high(*sem_led);
      delay__ms(100);
      gpiox__set_low(*sem_led);
      delay__ms(100); // blinking LED
      gpiox__set_high(*sem_led);
      delay__ms(100);
      gpiox__set_low(*sem_led);
      delay__ms(100); // blinking LED
      // vTaskDelay(100);
    }
    // Use xSemaphoreTake with forever delay and blink an LED when you get the signal
  }
}

void main_func(void *p) {
  const port_pin_s *sem_led = (port_pin_s *)(p);
  while (1) {
    // delay__ms(100);
    // T/ODO: Toggle an LED here
    gpiox__set_high(*sem_led);
    delay__ms(250);
    gpiox__set_low(*sem_led);
    delay__ms(250);
  }
}
#endif

/// lab 4 part 0
#if 0
 void pwm_task(void *p) {
  pwm1__init_single_edge(1000);

  // Locate a GPIO pin that a PWM channel will control
  // NOTE You can use gpio__construct_with_function() API from gpio.h
  // TO/DO Write this function yourself
  gpio__construct_with_function(GPIO__PORT_2, 0, 1);

  // We only need to set PWM configuration once, and the HW will drive
  // the GPIO at 1000Hz, and control set its duty cycle to 50%
  pwm1__set_duty_cycle(PWM1__2_0, 50);

  // Continue to vary the duty cycle in the loop
  uint8_t percent = 0;
  while (1) {
    pwm1__set_duty_cycle(PWM1__2_0, percent);

    if (++percent > 100) {
      percent = 0;
    }
    vTaskDelay(100);
  }
}
#endif

/// lab 4 part 1
#if 0
void pin_configure_adc_channel_as_io_pin() {
  LPC_IOCON->P1_31 &= ~(1 << 7); // set pin to Analog mode
  LPC_IOCON->P1_31 &= ~(7 << 0); // reset IOCON mux
  LPC_IOCON->P1_31 &= (3 << 0);  // route this pin as ADC channel 5
  // gpio__construct_with_function(GPIO__PORT_1, 31, GPIO__FUNCTION_3); /// LPC_IOCON->P1_31 &= ~(1 << 7);
}
void adc_task(void *p) {
  adc__initialize();

  // TODO This is the function you need to add to adc.h
  // You can configure burst mode for just the channel you are using
  adc__enable_burst_mode(1);
  adc__set_active_channel(ADC__CHANNEL_5);
  // LPC_ADC->CR &= ~(0xFF << 0); // clear all channel.

  // Configure a pin, such as P1.31 with FUNC 011 to route this pin as ADC channel 5
  // You can use gpio__construct_with_function() API from gpio.h
  pin_configure_adc_channel_as_io_pin(); // TODO You need to write this function

  while (1) {
    // Get the ADC reading us
    // ing a new routine you created to read an ADC burst reading
    // TODO: You need to write the implementation of this function
    const uint16_t adc_value = adc__get_channel_reading_with_burst_mode(ADC__CHANNEL_5);
    const double adc_voltage = (double)(adc_value) / 4095 * 3.3;
    fprintf(stderr, "ADC value is: %d， ADC voltage is: %.2f v\n", adc_value, adc_voltage);
    vTaskDelay(500);
  }
}
#endif

/// lab 4 part 2
#if 1
// This is the queue handle we will need for the xQueue Send/Receive API
static QueueHandle_t adc_to_pwm_task_queue; // important
void pin_configure_adc_channel_as_io_pin() {
  LPC_IOCON->P1_31 &= ~(1 << 7); // set pin to Analog mode
  LPC_IOCON->P1_31 &= ~(7 << 0); // reset IOCON mux
  LPC_IOCON->P1_31 &= (3 << 0);  // route this pin as ADC channel 5
  // gpio__construct_with_function(GPIO__PORT_1, 31, GPIO__FUNCTION_3); /// LPC_IOCON->P1_31 &= ~(1 << 7);
}
void adc_task(void *p) {
  // NOTE: Reuse the code from Part 1
  adc__initialize();
  adc__enable_burst_mode(1);
  pin_configure_adc_channel_as_io_pin();
  adc__set_active_channel(ADC__CHANNEL_5);
  int adc_reading = 0; // Note that this 'adc_reading' is not the same variable as the one from adc_task
  while (1) {
    // Implement code to send potentiometer value on the queue
    // a) read ADC input to 'int adc_reading'
    adc_reading = adc__get_channel_reading_with_burst_mode(ADC__CHANNEL_5);
    // b) Send to queue: xQueueSend(adc_to_pwm_task_queue, &adc_reading, 0);
    xQueueSend(adc_to_pwm_task_queue, &adc_reading, 0);
    vTaskDelay(100);
  }
}

void pwm_task(void *p) {
  // NOTE: Reuse the code from Part 0
  pwm1__init_single_edge(1000);
  gpio__construct_with_function(GPIO__PORT_2, 0, 1);
  LPC_IOCON->P2_0 &= ~(1 << 7);
  pwm1__set_duty_cycle(PWM1__2_0, 10);
  // uint8_t percent = 0;
  int adc_reading = 0;

  /// extra credit light up RGB LED
  gpio__construct_with_function(GPIO__PORT_2, 1, 1); // configure LPC_P2_1 to be PWN
  LPC_IOCON->P2_1 &= ~(1 << 7);                      // set pin to Analog mode
  gpio__construct_with_function(GPIO__PORT_2, 2, 1); // configure LPC_P2_2 to be PWN
  LPC_IOCON->P2_2 &= ~(1 << 7);                      // set pin to Analog mode
  pwm1__set_duty_cycle(PWM1__2_1, 10);
  pwm1__set_duty_cycle(PWM1__2_2, 10); // configure duty cycle
  static double red = 0, green = 0, blue = 0;
  static double percent = 0;
  // static int duty_cycle = 0;

  while (1) {
    // Implement code to receive potentiometer value from queue
    if (xQueueReceive(adc_to_pwm_task_queue, &adc_reading, 500)) {
      const double adc_voltage = (double)(adc_reading) / 4095 * 3.3;
      fprintf(stderr, "ADC value is: %d， ADC voltage is: %.2f v\n", adc_reading, adc_voltage);
      percent = (double)(adc_reading) / 4095 * 100;
      pwm1__set_duty_cycle(PWM1__2_0, percent);
      int duty = percent;
      /*if (percent <= 30) {
        red = percent / 30 * 100;
      } else if (percent < 60 && percent > 30) {
        red = 100;
        green = percent / 60 * 100;
      } else if (percent <= 100 && percent >= 60) {
        red = 100;
        green = 100;
        blue = (percent - 60) / 40 * 100;
      }*/
      red = percent;
      green = (1 - percent / 100) * 100;
      blue = percent / 1.5;
      /*int R[] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 240, 220, 200, 180,
                 160, 140, 120, 100, 80,  60,  40,  20,  0,   0,   0,   0,   0,   0,   0,   0,   0,
                 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                 0,   0,   20,  40,  60,  80,  100, 120, 140, 160, 180, 200, 220, 240};
      int G[] = {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   20,  40,  60,  80,  100, 120, 140,
                 160, 180, 200, 220, 240, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
      int B[] = {0,   20,  40,  60,  80,  100, 120, 140, 160, 180, 200, 220, 240, 255, 255, 255, 255,
                 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                 255, 255, 255, 255, 255, 240, 220, 200, 180, 160, 140, 120, 100, 80,  60,  40,  20,
                 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0};
      red = (R[duty] / 255) * 100;
      green = (G[duty] / 255) * 100;
      blue = (B[duty] / 255) * 100;
      fprintf(stderr, " duty number is: %.2d\n", duty);*/
      fprintf(stderr, " current duty cycle is: %.2f%%, R:%.2f, G:%.2f, B:%.2f\n", percent, red, green, blue);
      pwm1__set_duty_cycle(PWM1__2_0, red);
      pwm1__set_duty_cycle(PWM1__2_1, green);
      pwm1__set_duty_cycle(PWM1__2_2, blue);
    } else {
      puts("Timeout --> No data received");
    }
    // We do not need task delay because our queue API will put task to sleep when there is no data in the queue
    // vTaskDelay(500);
  }
}
#endif

/// lab 5 part 0
#if 0
// function writen in SSP2.C and SSP2.h
#endif

/// lab 5 part 1
#if 0

void spi_task(void *p) {
  const uint32_t spi_clock_mhz = 12;
  ssp2__lab_init(spi_clock_mhz);

  // From the LPC schematics pdf, find the pin numbers connected to flash memory
  // Read table 84 from LPC User Manual and configure PIN functions for SPI2 pins
  // You can use gpio__construct_with_function() API from gpio.h
  ssp2__init_spi_pins();
  // Note: Configure only SCK2, MOSI2, MISO2.
  // CS will be a GPIO output pin(configure and setup direction)
  // todo_configure_your_ssp2_pin_functions();

  while (1) {
    adesto_flash_id_s id = adesto_read_signature();
    // TODO: printf the members of the 'adesto_flash_id_s' struct
    fprintf(stderr, "manufacturer_id: %p, device_id1: %p, device_id2: %p, external_device_id: %p\n", id.manufacturer_id,
            id.device_id_1, id.device_id_2, id.extended_device_id);
    vTaskDelay(1000);
  }
}
#endif

/// lab 5 part 2
#if 0
xSemaphoreHandle spi_id_mutex_handler;

void spi_id_verification_task(void *p) {
  while (1) {
    if (xSemaphoreTake(spi_id_mutex_handler, 1000)) {
      const adesto_flash_id_s id = adesto_read_signature();

      // When we read a manufacturer ID we do not expect, we will kill this task
      if (id.manufacturer_id != 0x1F) {
        fprintf(stderr, "Manufacturer ID read failure :( \n");
        vTaskSuspend(NULL); // Kill this task
      } else {
        fprintf(stderr, "task 1 active: manufacturer_id: %p, device_id1: %p, device_id2: %p, external_device_id: %p\n",
                id.manufacturer_id, id.device_id_1, id.device_id_2, id.extended_device_id);
        // vTaskDelay(1000);
      }
      xSemaphoreGive(spi_id_mutex_handler);
    }
  }
}

void spi_id_verification_task_2(void *p) {
  while (1) {
    if (xSemaphoreTake(spi_id_mutex_handler, 1000)) {
      const adesto_flash_id_s id = adesto_read_signature();

      // When we read a manufacturer ID we do not expect, we will kill this task
      if (id.manufacturer_id != 0x1F) {
        fprintf(stderr, "Manufacturer ID read failure\n");
        vTaskSuspend(NULL); // Kill this task
      } else {
        fprintf(stderr, "task 2 active: manufacturer_id: %p, device_id1: %p, device_id2: %p, external_device_id: %p\n",
                id.manufacturer_id, id.device_id_1, id.device_id_2, id.extended_device_id);
        // vTaskDelay(1000);
      }
      xSemaphoreGive(spi_id_mutex_handler);
    }
  }
}

// void spi_id_verification_task_2(void *p) { xSemaphoreGive(spi_id_mutex_handler); }

#endif

/// lab 5 part 3 (extra credit)
#if 0

void spi_flash_read_page(void) {
  const uint32_t spi_clock_mhz = 12;
  ssp2__lab_init(spi_clock_mhz);
  ssp2__init_spi_pins();

  uint32_t data_address = 0x005C00;
  uint8_t input_data = 0xC1;
  int write_times = 10;

  while (1) {
    // delay__ms(10);
    const adesto_flash_id_s id = adesto_read_signature();

    // When we read a manufacturer ID we do not expect, we will kill this task
    if (id.manufacturer_id != 0x1F) {
      fprintf(stderr, "Manufacturer ID read failure\n");
      vTaskSuspend(NULL); // Kill this task
    } else {
      fprintf(stderr, "manufacturer_id: %p, device_id1: %p, device_id2: %p, external_device_id: %p\n",
              id.manufacturer_id, id.device_id_1, id.device_id_2, id.extended_device_id);
      // adesto_cs();

      // adesto_cs();
      adesto_write_enable();
      vTaskDelay(1);

      // flash_erase_chip();
      flash_erase_page(data_address);

      adesto_write_enable();
      vTaskDelay(1);

      uint8_t status;
      status = adesto_read_status();
      fprintf(stderr, "device status is: 0x%X \n", status);

      write_to_flash_8bitdata(data_address, input_data, write_times);
      // vTaskDelay(1);
      // ssp2__lab_exchange_byte(input_data);
      adesto_write_disable();
      vTaskDelay(1);
      fprintf(stderr, "data write start from location 0x%X is: 0x%X, for %d times\n", data_address, input_data,
              write_times);

      uint8_t status_0;
      status_0 = adesto_read_status();
      fprintf(stderr, "device status is: 0x%X \n", status_0);

      // start to read the flash
      // adesto_cs();
      uint8_t *result_arrary = adesto_read_arrary_address_input(data_address);
      // vTaskDelay(1);
      // adesto_ds();
      fprintf(stderr,
              "data read start from location 0x%X is: 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X\n",
              data_address, result_arrary[0], result_arrary[1], result_arrary[2], result_arrary[3], result_arrary[4],
              result_arrary[5], result_arrary[6], result_arrary[7], result_arrary[8], result_arrary[9]);
      fprintf(stderr, " \n");
      // fprintf(stderr, "data read from location is: %p\n", array_read);
      vTaskDelay(1000);
    }
  }
}

//}
#endif

/// lab 6 part 1
#if 0
void uart_read_task(void *p) {
  while (1) {
    // TODO: Use uart_lab__polled_get() function and printf the received value
    char data_read_from_poll = 'a';
    vTaskDelay(100);
    uint8_t as;
    uart_lab__polled_get(UART_3, &data_read_from_poll);
    fprintf(stderr, "data read from UART poll is: %c \n", data_read_from_poll);
    vTaskDelay(500);
  }
}

void uart_write_task(void *p) {
  while (1) {
    // TODO: Use uart_lab__polled_put() function and send a value
    char data_write_to_poll = '$';
    vTaskDelay(1000);
    uart_lab__polled_put(UART_3, data_write_to_poll);
    // vTaskDelay(500);
  }
}
#endif

/// lab 6 part 2
#if 1
void uart_interrupt_task(void *p) {
  const char data_get_from_uart_queue;
  while (1) {
    if (uart_lab__get_char_from_queue(&data_get_from_uart_queue, 500)) {
      fprintf(stderr, "data: %c\n", data_get_from_uart_queue);
    } else
      fprintf(stderr, "no data in the queue\n");
  }
}
#endif

/// lab 6 part 3
#if 1
xSemaphoreHandle uart_TR_handler;
// This task is done for you, but you should understand what this code is doing
void board_1_sender_task(void *p) {

  char number_as_string[16] = {0};

  while (true) {
    // if (xSemaphoreTake(uart_TR_handler, 99999)) {
    const int number = rand();
    sprintf(number_as_string, "%i", number);

    // Send one char at a time to the other board including terminating NULL char
    for (int i = 0; i <= strlen(number_as_string); i++) {
      uart_lab__polled_put(UART_3, number_as_string[i]);
      printf("Sent: %c\n", number_as_string[i]);
    }
    // vTaskDelay(1000);
    printf("Sent: %i over UART to the other board\n", number);
    vTaskDelay(3000);
    // xSemaphoreGive(uart_TR_handler);
    //}
  }
}

void board_2_receiver_task(void *p) {
  char number_as_string[16] = {0};
  int counter = 0;
  while (true) {
    // if (xSemaphoreTake(uart_TR_handler, 99999)) {
    char byte = 0;
    // vTaskDelay(50);
    uart_lab__get_char_from_queue(&byte, 9999);
    vTaskDelay(50);
    printf("Received: %c\n", byte);

    // This is the last char, so print the number
    if ('\0' == byte) {
      number_as_string[counter] = '\0';
      counter = 0;
      printf("Received this number from the other board: %s\n \n", number_as_string);
    }
    // We have not yet received the NULL '\0' char, so buffer the data
    else {
      number_as_string[counter] = byte;
      if (counter < 16) {
        counter++;
      } else {
        printf("reaching FIFO size limit \n");
      }
      // Hint: Use counter as an index, and increment it as long as we do not reach max value of 16
    }
    vTaskDelay(50);
    // xSemaphoreGive(uart_TR_handler);
    //}
  }
}
#endif

/// Producer Consumer Tasks
#if 0
static QueueHandle_t switch_queue;

typedef enum { switch__off, switch__on } switch_e;

switch_e get_switch_input_from_switch0(port_pin_s pin_num) {
  port_pin_s led0 = {2, 3};
  gpiox__set_as_input(pin_num);
  gpiox__set_as_output(led0);
  if (gpiox__get_level(pin_num)) {
    gpiox__set_low(led0);
    vTaskDelay(50);
    gpiox__set_high(led0);
    vTaskDelay(50);
    gpiox__set_low(led0);
    vTaskDelay(50);
    return switch__on;
  } else {
    return switch__off;
  }
}

// TODO: Create this task at PRIORITY_LOW
void producer(void *p) {
  static port_pin_s test_switch = {1, 19};
  while (1) {
    // This xQueueSend() will internally switch context to "consumer" task because it is higher priority than this
    // "producer" task Then, when the consumer task sleeps, we will resume out of xQueueSend()and go over to the next
    // line

    // TODO: Get some input value from your board
    const switch_e switch_value = get_switch_input_from_switch0(test_switch);

    // TODO: Print a message before xQueueSend()
    fprintf(stderr, "producer: before xqueuesend...\n");
    // Note: Use printf() and not fprintf(stderr, ...) because stderr is a polling printf
    xQueueSend(switch_queue, &switch_value, 1000);
    // this task will sleep for up to 0 ticks if the queue is full
    // TODO: Print a message after xQueueSend()
    fprintf(stderr, "producer: after xqueuesend...\n");
    fprintf(stderr, "during 1 second delay-->\n");
    // vTaskDelay(1000);
  }
}

// TODO: Create this task at PRIORITY_HIGH
void consumer(void *p) { //先运行
  switch_e switch_value;
  while (1) {
    // TODO: Print a message before xQueueReceive()
    // vTaskDelay(50);
    fprintf(stderr, "consumer: before xqueuerecieve...\n");
    xQueueReceive(switch_queue, &switch_value, 500);
    // this task will sleep for up to portMAX_DELAY until there is an item in the queue
    // vTaskDelay(50);
    // TODO: Print a message after xQueueReceive()
    fprintf(stderr, "consumer: switch value get from queue is: %d\n \n", switch_value);
  }
}
#endif

/// lab 7 CLI task
#if 1
void cli_led(void *p) {
  const port_pin_s *led_num = (port_pin_s *)(p);
  gpiox__set_as_output(*led_num);
  while (1) {
    gpiox__set_low(*led_num);
    vTaskDelay(200);
    gpiox__set_high(*led_num);
    vTaskDelay(200);
  }
}
#endif

/// midterm question
#if 1
QueueHandle_t q;
void producer(void *p) {
  int x;
  while (1) {
    xQueueSend(q, &x, 0);
    vTaskDelay(100);
  }
}

void consumer(void *p) {
  int y;
  while (1) {
    xQueueReceive(q, &y, 100);
    printf("Received %d\n", y);
  }
}
#endif

/// watch dog
#if 1
static QueueHandle_t watchdog_queue;
static EventGroupHandle_t watchdog_acc_sensor;
static UINT total_bytes_write = 0;
// uint16_t ticks = xTaskGetTickCount();

typedef struct {
  int16_t x, y, z;
} average_acc;

#define bit_id_producer_task (1 << 0)
void producer_task(void *params) {
  while (1) { // Assume 100ms loop - vTaskDelay(100)
    // Sample code:
    // 1. get_sensor_value()
    acceleration__axis_data_s acc_sensor_data;
    acceleration__init();
    acc_sensor_data = acceleration__get_data();
    acceleration__axis_data_s average_acc_data;
    for (int i = 0; i < 100; i++) {
      average_acc_data.x += acc_sensor_data.x;
      average_acc_data.y += acc_sensor_data.y;
      average_acc_data.z += acc_sensor_data.z;
      vTaskDelay(1);
    }
    average_acc_data.x /= 100;
    average_acc_data.y /= 100;
    average_acc_data.z /= 100;
    // average_acc_data.tick = xTaskGetTickCount();
    // fprintf(stderr, "Average accleration is:\n X axis: %i\n Y axis: %i\n Z axis: %i\n", average_acc_data.x,
    //         average_acc_data.y, average_acc_data.z);
    // 2. xQueueSend(handle, &sensor_value, 0);
    xQueueSend(watchdog_queue, &average_acc_data, 0);
    // 3. xEventGroupSetBits(checkin)
    // xEventGroupSetBits(watchdog_acc_sensor, bit_id_producer_task);
    // 4. vTaskDelay(100)
    // vTaskDelay(100);
    xEventGroupSetBits(watchdog_acc_sensor, bit_id_producer_task);
  }
}

#define bit_id_consumer_task (1 << 1)
void consumer_task(void *params) {
  // port_pin_s sd_cs = {1, 8};
  // gpiox__set_as_output(sd_cs);
  // gpiox__set_low(sd_cs);
  acceleration__axis_data_s average_acc_data_from_queue;
  acceleration__axis_data_s acc_data_array[10];
  int ticks_array[11];
  const *acc_data_file_name = "sensor_data_sd.txt";
  FIL acc_file; // File handle
  UINT bytes_written = 0;
  // UINT total_bytes = 0;
  int data_size = 0;

  while (1) { // Assume 100ms loop
    // No need to use vTaskDelay() because the consumer will consume as fast as production rate
    // because we should block on xQueueReceive(&handle, &item, portMAX_DELAY);
    // Sample code:
    // 1. xQueueReceive(handle, &sensor_value, portMAX_DELAY); // Wait forever for an item
    if (xQueueReceive(watchdog_queue, &average_acc_data_from_queue, 1)) {
      // fprintf(stderr, "%i", LPC_PCSDC);
      // Lpc_peripheral_power_control(LPC_PCSDC, 1);
      // data_size++;
      // printf("%i", data_size);
      if (data_size < 10) { // when data size < 10, go 10 times xQueueRecive to get data
        ticks_array[data_size] = xTaskGetTickCount();
        acc_data_array[data_size] = average_acc_data_from_queue; // storing the data get from queue for 10 times
        data_size++;
        // printf("%i", data_size);
      } else { // after 10th store of Average accleration sensor data, go write them in the sd card
        // fprintf(stderr, "Average accleration is:\n X axis: %i\n Y axis: %i\n Z axis: %i\n",
        //         average_acc_data_from_queue.x, average_acc_data_from_queue.y, average_acc_data_from_queue.z);
        FRESULT result = f_open(&acc_file, acc_data_file_name, (FA_WRITE | FA_CREATE_ALWAYS));

        // char string[512] = "0";
        // int string_index = 0;
        // for (int i = 0; i < 10; i++) { // store the array that contains 10 acc_sensor data into a string using
        // sprintf
        //   string_index += sprintf(&string[string_index], "Ticks: %i, X: %i, Y: %i, Z: %i \n", ticks_array[i],
        //                           acc_data_array[i].x, acc_data_array[i].y, acc_data_array[i].z);
        // }
        // fprintf(stderr, "%s \n", string);
        // write_to_sd(string);

        if (FR_OK == result) {
          char string[512] = "0";
          int string_index = 0;
          for (int i = 0; i < 10; i++) { // store the array that contains 10 acc_sensor data into a string using
            string_index += sprintf(&string[string_index], "Ticks: %lu, X: %i, Y: %i, Z: %i \n", xTaskGetTickCount(),
                                    acc_data_array[i].x, acc_data_array[i].y, acc_data_array[i].z);
          }
          // string[11] = "\n";
          if (f_lseek(&acc_file, total_bytes_write) == FR_OK) { /* keep aiming the end of the file to append data */
            fprintf(stderr, "%s \n", string);
            const int fw_status =
                f_write(&acc_file, string, strlen(string), &bytes_written); // write the string to the sd card
            // fprintf(stderr, "%i bytes has been written, the file write status is: %i\n", bytes_written, fw_status);
            if (FR_OK == fw_status) {
            } else {
              printf("ERROR: Failed to write data to file\n");
            }
            total_bytes_write += bytes_written; // update the total bytes written
            f_close(&acc_file);
          }
        } else {
          printf("ERROR: Failed to open: %s\n", acc_data_file_name);
        }

        data_size = 0;
      }
    }
    // fprintf(stderr, "!");
    xEventGroupSetBits(watchdog_acc_sensor, bit_id_consumer_task);
  }
}

#define watchdog_tasks_id bit_id_producer_task | bit_id_consumer_task
void watchdog_task(void *params) {
  while (1) {
    // ...
    // vTaskDelay(200);
    // We either should vTaskDelay, but for better robustness, we should
    // block on xEventGroupWaitBits() for slightly more than 100ms because
    // of the expected production rate of the producer() task and its check-in
    const *acc_data_file_name = "sensor_data_sd.txt";
    FIL acc_file; // File handle
    UINT bytes_written = 0;
    char string_watchdog_stauts[64] = "0";
    const EventBits_t watchdog_result = // the xEventGroupWaitBits(); will return certain value based on the check in
                                        // status of two watched tasks
        xEventGroupWaitBits(
            watchdog_acc_sensor, // event group handle
            watchdog_tasks_id,   // both porducer and consumer task bit will be tested
            pdTRUE,              // clear the bits that sets during the xEventGroupWaitBits(); will be cleared
            pdFALSE,             // this task will return only when both producer and consumer tasks' bits are tested
            150);
    /// fprintf(stderr, "watchdog result: %ld \n", watchdog_result);
    switch (watchdog_result) {
    case 0:
      sprintf(string_watchdog_stauts, "Ticks: %i, System halt, all task have been suspended... \n",
              xTaskGetTickCount());
      fprintf(stderr, "%s", string_watchdog_stauts);
      write_to_sd(string_watchdog_stauts);
      break;

    case 1:
      sprintf(string_watchdog_stauts, "Ticks: %i, Alert, consumer task failed to check in... \n", xTaskGetTickCount());
      fprintf(stderr, "%s", string_watchdog_stauts);
      write_to_sd(string_watchdog_stauts);
      break;

    case 2:
      sprintf(string_watchdog_stauts, "Ticks: %i, Alert, producer task failed to check in... \n", xTaskGetTickCount());
      fprintf(stderr, "%s", string_watchdog_stauts);
      write_to_sd(string_watchdog_stauts);
      break;

    case 3:
      sprintf(string_watchdog_stauts, "Ticks: %i, System is healthy... \n", xTaskGetTickCount());
      fprintf(stderr, "%s", string_watchdog_stauts);
      write_to_sd(string_watchdog_stauts);
      break;
    }
    vTaskDelay(1000);
  }
}

void write_to_sd(char string_watchdog_stauts[128]) {
  const *filename = "sensor_data_sd.txt";
  FIL file; // File handle
  UINT bytes_written = 0;
  FRESULT result = f_open(&file, filename, (FA_OPEN_APPEND | FA_WRITE)); // read/write pointer is set end of the file.
  if (f_lseek(&file, total_bytes_write) == FR_OK) {
    if (FR_OK == result) {
      // char string[256];
      // sprintf(string, "Alert, producer task failed to check in...\n");
      if (FR_OK == f_write(&file, string_watchdog_stauts, strlen(string_watchdog_stauts), &bytes_written)) {
        // fprintf(stderr, "%i bytes dog status has been written to %s\n", bytes_written, filename);
      } else {
        printf("ERROR: Failed to write data to file\n");
      }
      total_bytes_write += bytes_written;
      f_close(&file);
    } else {
      printf("ERROR: Failed to open: %s\n", filename);
    }
  }
}
#endif

int main(void) {
/// lab 2 part 0, 1
#if 0
  xTaskCreate(lab2_led_task, "LED", 1024 / sizeof(void *), NULL, 1, NULL);
  vTaskStartScheduler();
#endif

/// lab 2 part 2
#if 0
  // This is static such that these variables will be allocated in RAM and not go out of scope
  static port_pin_s led0 = {2, 3};
  static port_pin_s led1 = {1, 26};
  xTaskCreate(lab2_led_task0, "LED0", 1024 / sizeof(void *), &led0, 1, NULL); // &led0 is a task parameter going to
  // vTaskDelay(1500);
  xTaskCreate(lab2_led_task1, "LED1", 1024 / sizeof(void *), &led1, 1, NULL);
  // vTaskDelay(1500);
  sj2_cli__init();
  vTaskStartScheduler();
  // return 0;
#endif

/// lab 2 part 3
#if 0
  switch_press_indication = xSemaphoreCreateBinary();
  static port_pin_s test_switch = {0, 30}; // SW
  static port_pin_s test_led = {1, 24};    // LED
  // printf("level: %d", gpiox__get_level(test_switch)) 
  xTaskCreate(switch_task, "test_switch", 1024 / sizeof(void *), &test_switch, 1, NULL);
  // xTaskCreate(switch_task, "test_switch", configMINIMAL_STACK_SIZE, (void *)&test_switch, 1, NULL);
  xTaskCreate(lab2_led_task, "test_led", 1024 / sizeof(void *), &test_led, 1, NULL);
  // xTaskCreate(lab2_led_task, "test_led", , (void *)&test_led, 1, NULL);
  vTaskStartScheduler();configMINIMAL_STACK_SIZE
  return 0;
#endif

/// lab 3 part 0
#if 0
  /// Read Table 95 in the LPC user manual and setup an interrupt on a switch connected to Port0 or Port2
  /// a) For example, choose SW2 (P0_30) pin on SJ2 board and configure as input
  ///Warning: P0.30, and P0.31 require pull-down resistors
  static port_pin_s test_switch = {0, 29}; // SW
  static port_pin_s test_led = {1, 24};    // LED
  gpiox__set_as_input(test_switch);
  // b) Configure the registers to trigger Port0 interrupt (such as falling edge)
  gpiox__trigger_level(test_switch, 0); // set as triggering at the falling edge
  // Install GPIO interrupt function at the CPU interrupt (exception) vector
  // c) Hijack the interrupt vector at interrupt_vector_table.c and have it call our gpio_interrupt()
  //    Hint: You can declare 'void gpio_interrupt(void)' at interrupt_vector_table.c such that it can see this function
  // Most important step: Enable the GPIO interrupt exception using the ARM Cortex M API (this is from lpc40 qxx.h)
  NVIC_EnableIRQ(GPIO_IRQn);
  lpc_peripheral__enable_interrupt(GPIO_IRQn, gpio_interrupt); // gpio_interrupt = ISR
  // Toggle an LED in a loop to ensure/test that the interrupt is entering ane exiting
  // For example, if the GPIO interrupt gets stuck, this LED will stop blinking
  // vTaskStartScheduler();
  while (1) {
    delay__ms(100);
    // T/ODO: Toggle an LED here
    gpiox__set_high(test_led);
    delay__ms(500);
    gpiox__set_low(test_led);
    delay__ms(500);
  }
  // vTaskStartScheduler();
  // return 0;
#endif

/// lab 3 part 1
#if 0
  switch_pressed_signal = xSemaphoreCreateBinary(); // Create your binary semaphore
  static port_pin_s test_switch2 = {0, 30};
  static port_pin_s test_led2 = {1, 24};
  gpiox__set_as_input(test_switch2);
  gpiox__trigger_level(test_switch2, 0);
  // configure_your_gpio_interrupt(); // T/ODO: Setup interrupt by re-using code from Part 0
  NVIC_EnableIRQ(GPIO_IRQn); // Enable interrupt gate for the GPIO
  lpc_peripheral__enable_interrupt(GPIO_IRQn, gpio_interrupt2, "unused");
  xTaskCreate(sleep_on_sem_task, "sleep_sem", (512U * 4) / sizeof(void *), &test_led2, 1, NULL);
  vTaskStartScheduler();
#endif

/// lab 3 part 2
#if 0
  /* SW0: P1_19; LED0: P2_3;
   * SW1: P1_15; LED1: P1_26;
   * SW2: P0_30; LED2: P1_24;
   * SW3: P0_29; LED3: P1_18;
   */
  switch_pressed_signal_hw3_part2_30 = xSemaphoreCreateBinary();
  switch_pressed_signal_hw3_part2_31 = xSemaphoreCreateBinary();
  static port_pin_s part2_test_switch_2 = {0, 30};
  gpiox__set_as_input(part2_test_switch_2);
  static port_pin_s part2_test_switch_3 = {0, 29};
  gpiox__set_as_input(part2_test_switch_3);

  static port_pin_s test_led_main = {2, 3};
  gpiox__set_as_output(test_led_main);

  static port_pin_s part2_test_led_0 = {1, 26};
  static port_pin_s part2_test_led_1 = {1, 24};

  fprintf(stderr, "Entering...\n");
  gpiox__attach_interrupt(part2_test_switch_2, GPIO_INTR__RISING_EDGE, gpio_interrupt_part2_0);
  gpiox__attach_interrupt(part2_test_switch_3, GPIO_INTR__FALLING_EDGE, gpio_interrupt_part2_1);

  NVIC_EnableIRQ(GPIO_IRQn);
  lpc_peripheral__enable_interrupt(GPIO_IRQn, gpiox__interrupt_dispatcher, "unused");

  xTaskCreate(pin30_isr, "Interrupt 1", (512U * 4) / sizeof(void *), &part2_test_led_0, 2, NULL);
  xTaskCreate(pin31_isr, "Interrupt 2", (512U * 4) / sizeof(void *), &part2_test_led_1, 2, NULL);
  xTaskCreate(main_func, "main function", (512U * 4) / sizeof(void *), &test_led_main, 1, NULL);
  vTaskStartScheduler();
  return 0;
#endif

/// lab 4 part 0
#if 0
  xTaskCreate(pwm_task, "pwm_led", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  vTaskStartScheduler();
#endif

/// lab 4 part 1
#if 0
  xTaskCreate(adc_task, "adc_task", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  vTaskStartScheduler();
#endif

/// lab 4 part 2
#if 0
  // Queue will only hold 1 integer
  adc_to_pwm_task_queue = xQueueCreate(1, sizeof(int)); // important to do this initailize step

  xTaskCreate(adc_task, "adc_task_sending_to_pwm", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(pwm_task, "pwm_led", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  vTaskStartScheduler();
#endif

/// lab 5 part 0
#if 0
// function writen in SSP2.C and SSP2.h
#endif

/// lab 5 part 1
#if 0
  xTaskCreate(spi_task, "spi_id_task", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  vTaskStartScheduler();
#endif

/// lab 5 part 2
#if 0
  // TODO: Initialize your SPI, its pins, Adesto flash CS GPIO etc...
  spi_id_mutex_handler = xSemaphoreCreateMutex();
  const uint32_t spi_clock_mhz_part2 = 24;
  ssp2__lab_init(spi_clock_mhz_part2);
  // Create two tasks that will continously read signature
  xTaskCreate(spi_id_verification_task, "spi_id_guard1", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(spi_id_verification_task_2, "spi_id_guard2", (512U * 4) / sizeof(void *), NULL, 1, NULL);

  vTaskStartScheduler();

#endif

/// lab 5 part 3 (extra credit)
#if 0
  xTaskCreate(spi_flash_read_page, "spi_id_task", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  vTaskStartScheduler();
#endif

/// lab 6 part 1
#if 0
  // TODO: Use uart_lab__init() function and initialize UART2 or UART3 (your choice)
  // TODO: Pin Configure IO pins to perform UART2/UART3 function
  // fprintf(stderr, "core clk:%ld  per_clk: %ld\n", clock__get_core_clock_hz(), clock__get_peripheral_clock_hz());
  const uint32_t peripheral_clock = clock__get_peripheral_clock_hz();
  uart_lab__init(UART_2, peripheral_clock, 9600);
  uart_lab__init(UART_3, peripheral_clock, 9600);
  fprintf(stderr, "core clk:%ld  peripheral clk: %ld\n", clock__get_core_clock_hz(), clock__get_peripheral_clock_hz()); 

  // gpio__construct_with_function(GPIO__PORT_0, 10, GPIO__FUNCTION_1); // U2_TXD this one not works
  gpio__construct_with_function(GPIO__PORT_0, 11, GPIO__FUNCTION_1); // U2_RXD
  gpio__construct_with_function(GPIO__PORT_2, 8, GPIO__FUNCTION_2);  // u2_txd
  gpio__construct_with_function(GPIO__PORT_2, 9, GPIO__FUNCTION_2);  // u2_rxd

  gpio__construct_with_function(GPIO__PORT_0, 0, GPIO__FUNCTION_2);  // U3_TXD
  gpio__construct_with_function(GPIO__PORT_0, 1, GPIO__FUNCTION_2);  // U3_RXD
  gpio__construct_with_function(GPIO__PORT_0, 25, GPIO__FUNCTION_3); // u3_txd
  gpio__construct_with_function(GPIO__PORT_0, 26, GPIO__FUNCTION_3); // u3_rxd

  xTaskCreate(uart_read_task, "uart_read", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(uart_write_task, "uart_write", (512U * 4) / sizeof(void *), NULL, 1, NULL);

  vTaskStartScheduler();
#endif

/// lab 6 part 2 & 3
#if 0
  // fprintf(stderr, "core clk:%ld  per_clk: %ld\n", clock__get_core_clock_hz(), clock__get_peripheral_clock_hz());
  // uart_TR_handler = xSemaphoreCreateMutex();
  const uint32_t peripheral_clock = clock__get_peripheral_clock_hz();
  uart_lab__init(UART_2, peripheral_clock, 9600);
  uart_lab__init(UART_3, peripheral_clock, 9600);
  fprintf(stderr, "core clk:%ld  peripheral clk: %ld\n", clock__get_core_clock_hz(), clock__get_peripheral_clock_hz());

  // gpio__construct_with_function(GPIO__PORT_0, 10, GPIO__FUNCTION_1); // U2_TXD this one not works
  gpio__construct_with_function(GPIO__PORT_0, 11, GPIO__FUNCTION_1); // U2_RXD
  gpio__construct_with_function(GPIO__PORT_2, 8, GPIO__FUNCTION_2);  // u2_txd
  gpio__construct_with_function(GPIO__PORT_2, 9, GPIO__FUNCTION_2);  // u2_rxd

  gpio__construct_with_function(GPIO__PORT_0, 0, GPIO__FUNCTION_2);  // U3_TXD
  gpio__construct_with_function(GPIO__PORT_0, 1, GPIO__FUNCTION_2);  // U3_RXD
  gpio__construct_with_function(GPIO__PORT_0, 25, GPIO__FUNCTION_3); // u3_txd
  gpio__construct_with_function(GPIO__PORT_0, 26, GPIO__FUNCTION_3); // u3_rxd

  uart__enable_receive_interrupt(UART_2);
  uart__enable_receive_interrupt(UART_3);

  // xTaskCreate(uart_interrupt_task, "uart_read_interrupt", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  // xTaskCreate(uart_write_task, "uart_write", (512U * 4) / sizeof(void *), NULL, 1, NULL);

  xTaskCreate(board_1_sender_task, "uart_send", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(board_2_receiver_task, "uart_recive", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  sj2_cli__init();
  vTaskStartScheduler();
#endif

/// Producer Consumer Tasks
#if 0
  // TODO: Create your tasks
  TaskHandle_t consumer_task_handle;
  xTaskCreate(producer, "producer", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(consumer, "consumer", (512U * 4) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  // TODO Queue handle is not valid until you create it
  switch_queue =
      xQueueCreate(3, sizeof(switch_e)); // Choose depth of item being our enum (1 should be okay for this example)
  sj2_cli__init();
  vTaskStartScheduler();

#endif

/// lab 7 CLI task
#if 0

  peripherals_init__i2c1_init(0x86);
  static port_pin_s cli_task_led = {2, 3};
  xTaskCreate(cli_led, "cli_led", (512U * 4) / sizeof(void *), &cli_task_led, 1, NULL);

  sj2_cli__init();
  // vTaskResetRunTimeStats();
  vTaskStartScheduler();
#endif

/// midterm question
#if 0
  q = xQueueCreate(1, sizeof(int));
  xTaskCreate(producer, "producer", 1024, NULL, PRIORITY_LOW, NULL);
  xTaskCreate(consumer, "consumer", 1024, NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
#endif

/// watch dog
#if 0
  watchdog_queue = xQueueCreate(1, sizeof(acceleration__axis_data_s));
  watchdog_acc_sensor = xEventGroupCreate();
  xTaskCreate(producer_task, "wd_producer", (512U * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(consumer_task, "wd_consumer", (512U * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(watchdog_task, "doggie", (512U * 4) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  sj2_cli__init();
  vTaskStartScheduler();
#endif

/// LAB i2c
#if 1
  peripherals_init__i2c1_init(0x86);
  sj2_cli__init();
  static port_pin_s cli_task_led = {2, 3};
  xTaskCreate(cli_led, "cli_led", (512U * 4) / sizeof(void *), &cli_task_led, 1, NULL);
  vTaskStartScheduler();
#endif
}

// P3_15

// LPC_IOCON->P3_15 &= ~(7 << 0); ~0111 = 1000
// LPC_GPIO3->SET |= ( 1 << 15);
// LPC_GPIO3->CLR |= ( 1 << 15);

// LPC_SC->PCONP |= ( 1 << 24 );
// PCONP里面有32个bit，每一个bit控制了一个peripheral的电源

// ==, =, |=, &=,

// LPC_UART2->LRC = ( 1 << 7 ); 1 << 7 = 0000 0000 0000 0000 0000 0000 10000000
// LPC_UART2->LRC = 0000 0000 0000 0000 0000 0000 10000000

// LPC_UART2->LRC &= ~( 1 << 7 ); ~(1 << 7) = 1111 1111 1111 1111 1111 1111 01111111
// LPC_UART2->LRC &= 1111 1111 1111 1111 1111 1111 01111111

// LPC_UART2->LRC ^= (1 << 5); (1 << 5) = 10000
// LPC_UART2->LRC = (LPC_UART2->LRC) xor (10000)

// $s1 write only
// addi $t0, 5 (0101)
// orr $s1, $s1, $t0

// addi THR, output_byte

// LPC_SSP0->SR
// 96 0000 0000 hz = cpu clock
// 9600 = baud

// const uint16_t divider_16_bit = (96 0000 0000 / (16 * 9600));
