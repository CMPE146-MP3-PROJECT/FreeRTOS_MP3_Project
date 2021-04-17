#include "keypad.h"
//#include "gpio_lab.h"

static port_pin_s IO_to_key_pin1 = {0, 25}; // output
static port_pin_s IO_to_key_pin2 = {1, 31}; // output
static port_pin_s IO_to_key_pin3 = {1, 20}; // output
static port_pin_s IO_to_key_pin4 = {1, 28}; // output

static port_pin_s key_to_IO_pin1 = {2, 0};        // input
static port_pin_s key_to_IO_pin2 = {2, 2};        // input
static port_pin_s key_to_IO_pin3 = {2, 5};        // input
static port_pin_s key_to_IO_pin4 = {2, 7};        // input
static port_pin_s key_pad_interrupt_pin = {0, 1}; // keypad hw intr pin

void key_pins_init(void) {

  gpio__construct_with_function(GPIO__PORT_0, 1, GPIO__FUNCITON_0_IO_PIN);
  gpiox__set_as_input(key_pad_interrupt_pin);

  gpio__construct_with_function(GPIO__PORT_0, 25, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(GPIO__PORT_1, 31, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(GPIO__PORT_1, 20, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(GPIO__PORT_1, 28, GPIO__FUNCITON_0_IO_PIN);
  gpiox__set_as_output(IO_to_key_pin1);
  gpiox__set_as_output(IO_to_key_pin2);
  gpiox__set_as_output(IO_to_key_pin3);
  gpiox__set_as_output(IO_to_key_pin4);

  gpio__construct_with_function(GPIO__PORT_2, 0, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(GPIO__PORT_2, 2, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(GPIO__PORT_2, 5, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(GPIO__PORT_2, 7, GPIO__FUNCITON_0_IO_PIN);
  gpiox__set_as_input(key_to_IO_pin1);
  gpiox__set_as_input(key_to_IO_pin2);
  gpiox__set_as_input(key_to_IO_pin3);
  gpiox__set_as_input(key_to_IO_pin4);
}

void read_keys(void) {

  // while(1){
  // if (xSemaphoreTake(key_press_indication, portMAX_DELAY)) {
  char which_bottom = 0;
  char key[16] = {'D', '#', '0', '*', 'C', '9', '8', '7', 'B', '6', '5', '4', 'A', '3', '2', 'i'};
  int result_arrary[4];

  IO_pins_value_write(1);                      // write 1110 to keypad
  int *c1 = IO_pins_value_read(result_arrary); // read
  for (int i = 0; i < 4; i++) {
    if (c1[i] != 1) {
      // fprintf(stderr, "%c", key[0 + i]);
      which_bottom = key[0 + i];
      // fprintf(stderr, "!!!!! %c", c1);
      xQueueSend(Q_keypad_bottom, &which_bottom, 0);
      // break;
    }
  }

  IO_pins_value_write(2);                      // write 1101 to keypad
  int *c2 = IO_pins_value_read(result_arrary); // read
  for (int i = 0; i < 4; i++) {
    if (c2[i] != 1) {
      // fprintf(stderr, "%c", key[4 + i]);
      which_bottom = key[4 + i];
      xQueueSend(Q_keypad_bottom, &which_bottom, 0);
      // break;
    }
  }

  IO_pins_value_write(3);                      // write 1011 to keypad
  int *c3 = IO_pins_value_read(result_arrary); // read
  for (int i = 0; i < 4; i++) {
    if (c3[i] != 1) {
      // fprintf(stderr, "%c", key[8 + i]);
      which_bottom = key[8 + i];
      xQueueSend(Q_keypad_bottom, &which_bottom, 0);
      // break;
    }
  }

  IO_pins_value_write(4);                      // write 0111 to keypad
  int *c4 = IO_pins_value_read(result_arrary); // read from
  for (int i = 0; i < 4; i++) {
    if (c4[i] != 1) {
      // break;
    }
  }

  vTaskDelay(150);
  //}
  //}
}

void IO_pins_value_write(int write_state) {
  switch (write_state) {
  case 1: // 1110
    gpiox__set_high(IO_to_key_pin1);
    gpiox__set_high(IO_to_key_pin2);
    gpiox__set_high(IO_to_key_pin3);
    gpiox__set_low(IO_to_key_pin4);
    vTaskDelay(5);
    break;
  case 2: // 1101
    gpiox__set_high(IO_to_key_pin1);
    gpiox__set_high(IO_to_key_pin2);
    gpiox__set_low(IO_to_key_pin3);
    gpiox__set_high(IO_to_key_pin4);
    vTaskDelay(5);
    break;
  case 3: // 1011
    gpiox__set_high(IO_to_key_pin1);
    gpiox__set_low(IO_to_key_pin2);
    gpiox__set_high(IO_to_key_pin3);
    gpiox__set_high(IO_to_key_pin4);
    vTaskDelay(5);
    break;
  case 4: // 0111 (1111)
    gpiox__set_low(IO_to_key_pin1);
    gpiox__set_high(IO_to_key_pin2);
    gpiox__set_high(IO_to_key_pin3);
    gpiox__set_high(IO_to_key_pin4);
    vTaskDelay(5);
    break;
  }
}

int *IO_pins_value_read(int *result_arrary) {
  // static int result_arrary[4];
  result_arrary[0] = gpiox__get_level(key_to_IO_pin4);
  result_arrary[1] = gpiox__get_level(key_to_IO_pin3);
  result_arrary[2] = gpiox__get_level(key_to_IO_pin2);
  result_arrary[3] = gpiox__get_level(key_to_IO_pin1);
  // printf(stderr, "result arrary: %ls", result_arrary);
  return result_arrary;
  // https://www.awaimai.com/2819.html <<<-passing a array to a function
}