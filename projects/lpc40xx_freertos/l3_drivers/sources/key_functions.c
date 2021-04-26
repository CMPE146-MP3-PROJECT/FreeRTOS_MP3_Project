#include "key_functions.h"

/**************for task control*****************/
// TaskHandle_t song_reader_task;
// TaskHandle_t song_player_task;
// static bool play_pause_status = true;
/***********************************************/
/*************for volume control****************/
static int plus = 1;
static int minus = 0;
// static uint16_t volume_value = 0x2020;
// static uint16_t volume_mask = 0x0202;
/***********************************************/
/************for PLAYING/SKIPPING***************/

/***********************************************/
/**************keypad keys usage****************/
//    [vol-]       []       [vol+]    [bass+]
//
//      []    [play/pause]    []      [bass-]
//
//    [prev]       []       [next]    [vocal+]
//
//      []         []         []      [vocal-]
/***********************************************/
// uint16_t dummy_byte_16bits = 0x1234;
// uint16_t msb;
// uint8_t lsb;
// uint8_t abb;

songname_backup song_name = {0};

// key_functions_init() {
//   song_reader_task = xTaskGetHandle("send_song");
//   song_player_task = xTaskGetHandle("play_song");
// }

key_functions(char key_detected) {
  // key_functions_init();
  uint16_t test = 0x3333;
  // fprintf(stderr, "key: %c\n", key_detected);
  switch (key_detected) { // https://www.yiibai.com/book/article/820 <<-switch a char
  case '1':

    DAC_volume_control(minus);

    // volume_value = volume_value + volume_mask;
    // SCI_set_volume(volume_value);
    // fprintf(stderr, "  volume: L: -%ddB R: -%ddB\n", (volume_value >> 8), (volume_value & 0xFF));
    break;

  case '2':
    break;

  case '3':
    DAC_volume_control(plus);
    // if (volume_value == 0x1212) {
    //   fprintf(stderr, "  warning: too laud volume may damage your ear!\n");
    // } else {
    //   volume_value = volume_value - volume_mask;
    //   SCI_set_volume(volume_value);
    //   fprintf(stderr, "  volume: L: -%ddB R: -%ddB\n", (volume_value >> 8), (volume_value & 0xFF));
    // }
    break;

  case 'A': // increase bass
    DAC_bass_control(plus);
    break;

  case '4':
    break;

  case '5':
    DAC_play_pause();
    // if (play_pause_status) {
    //   play_pause_status = false;
    //   vTaskSuspend(song_reader_task);
    //   fprintf(stderr, "playback pause...\n");
    // } else {
    //   play_pause_status = true;
    //   vTaskResume(song_reader_task);
    //   fprintf(stderr, "playback resume...\n");
    // }
    break;

  case '6':
    break;
  case 'B': // decrease bass
    DAC_bass_control(minus);
    break;

  case '7':
    xSemaphoreGive(rewind_song);
    break;

  case '8':
    break;

  case '9':
    break;

  case 'C':
    DAC_treble_control(plus);
    break;

  case '*':
    break;

  case '0':
    // // SCI_read_volume();
    // adafruit_cs();
    // SCI_start_raed();
    // spi0__mp3_exchange_byte(0x02);
    // // abb = spi0__mp3_exchange_byte(0xFF);
    // uint16_t msb = spi0__mp3_exchange_byte(0xFF);
    // uint16_t lsb = spi0__mp3_exchange_byte(0xFF);
    // adafruit_ds();
    // fprintf(stderr, "  msb: 0x%X lsb:0x%X\n", msb, lsb);
    break;

  case '#':

    // strncpy(song_name, "world.execute.mp3", sizeof(songname_backup) - 1);
    // xQueueSend(Q_songname, &song_name, 0);
    break;

  case 'D':
    DAC_treble_control(minus);
    break;
  }
}
