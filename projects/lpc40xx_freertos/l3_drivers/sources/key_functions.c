#include "key_functions.h"

/**************for task control*****************/
TaskHandle_t song_reader_task;
TaskHandle_t song_player_task;
static bool play_pause_status = true;
/***********************************************/
/*************for volume control****************/
static int volume_up = 1;
static int volume_down = 0;
// static uint16_t volume_value = 0x2020;
// static uint16_t volume_mask = 0x0202;
/***********************************************/
/****************for SKIPPING*******************/
static bool SKIP_status = true;
/***********************************************/
/*************for bass control****************/
static uint16_t base_value = 0x0002;
static uint16_t bass_mask = 0x0010;
/***********************************************/

uint16_t dummy_byte_16bits = 0x1234;
uint16_t msb;
uint8_t lsb;
uint8_t abb;

key_functions_init() {
  song_reader_task = xTaskGetHandle("send_song");
  song_player_task = xTaskGetHandle("play_song");
}

key_functions(char key_detected) {
  key_functions_init();
  uint16_t test = 0x3333;
  // fprintf(stderr, "key: %c\n", key_detected);
  switch (key_detected) { // https://www.yiibai.com/book/article/820 <<-switch a char
  case '1':
    SCI_read_volume();
    // fprintf(stderr, "test printf: 0x%X", test);
    // adafruit_cs();
    // SCI_start_raed();
    // spi0__mp3_exchange_byte(0x0B);
    // abb = spi0__mp3_exchange_byte(0xFF);
    // msb = spi0__mp3_exchange_byte(0xFF);
    // lsb = spi0__mp3_exchange_byte(0xFF);
    // adafruit_ds();
    // fprintf(stderr, "  abb: 0x%X msb: 0x%X lsb:0x%X\n", abb, msb, lsb);
    break;
  case '2':
    break;
  case '3':
    DAC_volume_control(volume_up);
    // if (volume_value == 0x1212) {
    //   fprintf(stderr, "  warning: too laud volume may damage your ear!\n");
    // } else {
    //   volume_value = volume_value - volume_mask;
    //   SCI_set_volume(volume_value);
    //   fprintf(stderr, "  volume: L: -%ddB R: -%ddB\n", (volume_value >> 8), (volume_value & 0xFF));
    // }
    break;
  case 'A': // increase bass
    break;
  case '4':
    break;

  case '5':
    // DAC_play_pause();
    if (play_pause_status) {
      play_pause_status = false;
      vTaskSuspend(song_reader_task);
      fprintf(stderr, "playback pause...\n");
    } else {
      play_pause_status = true;
      vTaskResume(song_reader_task);
      fprintf(stderr, "playback resume...\n");
    }
    break;

  case '6':
    break;
  case 'B': // decrease bass
    break;

  case '7':
    xSemaphoreGive(next_song);
    // // not working
    // if (SKIP_status) {
    //   SKIP_status = false;
    //   SCI_set_CLOCKF(0xD8000);
    //   puts("skipping...");
    // } else {
    //   SKIP_status = true;
    //   SCI_set_CLOCKF(0xC000);
    //   puts("playing...");
    // }
    break;

  case '8':
    break;
  case '9':
    DAC_volume_control(volume_down);

    // volume_value = volume_value + volume_mask;
    // SCI_set_volume(volume_value);
    // fprintf(stderr, "  volume: L: -%ddB R: -%ddB\n", (volume_value >> 8), (volume_value & 0xFF));
    break;
  case 'C':
    break;
  case '*':
    break;
  case '0':
    break;
  case '#':
    break;
  case 'D':
    break;
  }
}