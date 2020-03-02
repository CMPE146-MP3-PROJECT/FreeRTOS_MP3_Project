#include "board_io.h"
#include "delay.h"
#include "gpio.h"

int main(void) {
  const gpio_s led = board_io__get_led0();

  while (true) {
    gpio__toggle(led);
    delay__ms(500);
  }

  return 1; // main() shall never return
}
