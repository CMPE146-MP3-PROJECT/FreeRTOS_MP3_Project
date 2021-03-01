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
#include "adc.h"
#include "gpio.h"
#include "periodic_scheduler.h"
#include "pwm1.h"
#include "queue.h"
#include "sj2_cli.h"
#include "ssp2.h"
#include "task.h"

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
static SemaphoreHandle_t switch_pressed_signal_hw3_part2_30;
static SemaphoreHandle_t switch_pressed_signal_hw3_part2_31;

/// lab 2 part 0
#if 0
void lab2_led_task(void *pvParameters) {
  // Choose one of the onboard LEDS by looking into schematics and write code for the below
  // 0) Set the IOCON MUX function(if required) select pins to 000
  LPC_IOCON->P2_3 &= (7 << 0);
  // 0x1000100101 & 0x000 = 0x1000100000
  // 1) Set the DIR register bit for the LED port pin, 1:output, 0:input
  LPC_GPIO2->DIR |= (1 << 4); //0001 << 4 = 10000;
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
#if 0
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
#if 0
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
void gpio_interrupt_part2_0(void) { xSemaphoreGiveFromISR(switch_pressed_signal_hw3_part2_30, NULL); }
void gpio_interrupt_part2_1(void) { xSemaphoreGiveFromISR(switch_pressed_signal_hw3_part2_31, NULL); }
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
  LPC_IOCON->P1_31 &= ~(7 << 0); // reset IOCON mux
  LPC_IOCON->P1_31 &= (3 << 0); // route this pin as ADC channel 5
                                // gpio__construct_with_function(GPIO__PORT_1, 31, 1); ///LPC_IOCON->P1_31 &= ~(1 << 7);
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
#if 0
// This is the queue handle we will need for the xQueue Send/Receive API
static QueueHandle_t adc_to_pwm_task_queue; // important
void pin_configure_adc_channel_as_io_pin() {
  LPC_IOCON->P1_31 &= ~(7 << 0); // reset IOCON mux
  LPC_IOCON->P1_31 &= (3 << 0);  // route this pin as ADC channel 5
                                // gpio__construct_with_function(GPIO__PORT_1, 31, 1); ///LPC_IOCON->P1_31 &= ~(1 << 7);
}
void adc_task(void *p) {
  // NOTE: Reuse the code from Part 1
  adc__initialize();
  adc__enable_burst_mode(1);
  adc__set_active_channel(ADC__CHANNEL_5);
  pin_configure_adc_channel_as_io_pin();
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
  pwm1__set_duty_cycle(PWM1__2_0, 10);
  // uint8_t percent = 0;
  int adc_reading = 0;

  /// extra credit light up RGB LED
  gpio__construct_with_function(GPIO__PORT_2, 1, 1); // configure LPC_P2_1 to be PWN
  gpio__construct_with_function(GPIO__PORT_2, 2, 1); // configure LPC_P2_2 to be PWN
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
      red = (G[duty] / 255) * 100;
      red = (B[duty] / 255) * 100;
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
#if 1

void spi_task(void *p) {
  const uint32_t spi_clock_mhz = 24;
  ssp2__lab_init(spi_clock_mhz);

  // From the LPC schematics pdf, find the pin numbers connected to flash memory
  // Read table 84 from LPC User Manual and configure PIN functions for SPI2 pins
  // You can use gpio__construct_with_function() API from gpio.h
  gpio__construct_with_function(GPIO__PORT_1, 0, GPIO__FUNCTION_4); // enable SSP2_SCK
  gpio__construct_with_function(GPIO__PORT_1, 1, GPIO__FUNCTION_4); // enable SSP2_MOSI
  gpio__construct_with_function(GPIO__PORT_1, 4, GPIO__FUNCTION_4); // enable SSP2_MISO
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
void spi_id_verification_task(void *p) {
  while (1) {
    const adesto_flash_id_s id = ssp2__adesto_read_signature();
    
    // When we read a manufacturer ID we do not expect, we will kill this task
    if (id.manufacturer_id != 0x1F) {
      fprintf(stderr, "Manufacturer ID read failure\n");
      vTaskSuspend(NULL); // Kill this task
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
  // xTaskCreate(lab2_led_task, "test_led", configMINIMAL_STACK_SIZE, (void *)&test_led, 1, NULL);
  vTaskStartScheduler();
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
  lpc_peripheral__enable_interrupt(GPIO_IRQn, gpio_interrupt2);
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
#if 1
  xTaskCreate(spi_task, "spi_id_task", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  vTaskStartScheduler();
#endif

/// lab 5 part 2
#if 0
  // TODO: Initialize your SPI, its pins, Adesto flash CS GPIO etc...

  // Create two tasks that will continously read signature
  xTaskCreate(spi_id_verification_task, ...);
  xTaskCreate(spi_id_verification_task, ...);

  vTaskStartScheduler();

#endif
}
