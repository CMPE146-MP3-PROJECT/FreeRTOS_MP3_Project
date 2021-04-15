#include "FreeRTOS.h"
#include "app_cli.h"
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
#include "mp3_codec_spi.h"
#include "periodic_scheduler.h"
#include "pwm1.h"
#include "queue.h"
#include "sj2_cli.h"
#include "ssp2.h"
#include "task.h"
#include "uart.h"
#include "uart_lab.h"

/******************************************************************************************/
/***************************************MP3 PROJECT****************************************/
#if 1
typedef char songname_t[16];
typedef char songname[32];
typedef char song_data_t[512];

#define SD_MP3_FILE_READ_WRITE_TEST 0
QueueHandle_t Q_songname;
QueueHandle_t Q_songdata;

app_cli_status_e cli__mp3_play(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                               app_cli__print_string_function cli_output) {
  // user_input_minus_command_name is actually a 'char *' pointer type
  // We tell the Queue to copy 32 bytes of songname from this location
  songname song_file_name = {0};

  strncpy(song_file_name, user_input_minus_command_name, sizeof(songname) - 1);
  printf("Sending input \"%s\" to queue\n", song_file_name);
  xQueueSend(Q_songname, song_file_name, portMAX_DELAY);
  return APP_CLI_STATUS__SUCCESS;
}

// Reader tasks receives song-name over Q_songname to start reading it
void mp3_reader_task(void *p) {
  songname song_file_name;
  song_data_t byte_512_song_data;
  const *acc_data_file_name = "sensor_data_sd.txt";
  // const *song_data_file_name = "bad.mp3";
  FIL song_file, acc_file; /* [IN] File object */
  void *buff;              /* [OUT] Buffer to store read data */
  UINT btr;                /* [IN] Number of bytes to read */
  UINT *br;                /* [OUT] Number of bytes read */
  UINT bytes_written = 0;
  FRESULT fr; /* FatFs function common result code */
  while (1) {
    if (xQueueReceive(Q_songname, &song_file_name, 1000)) {
      fprintf(stderr, "Received song to play: %s\n", song_file_name);
      fr = f_open(&song_file, song_file_name, FA_READ); // open_file();
      if (FR_OK == fr) {
        f_read(&song_file, byte_512_song_data, sizeof(byte_512_song_data), &br);
        fprintf(stderr, "song data: %s\n", byte_512_song_data);

        // xQueueSend(Q_songdata, &bytes_512[0], portMAX_DELAY);

#if SD_MP3_FILE_READ_WRITE_TEST
        if (FR_OK == f_open(&acc_file, acc_data_file_name, (FA_WRITE | FA_CREATE_ALWAYS))) {
          if (FR_OK == f_write(&acc_file, byte_512_song_data, sizeof(byte_512_song_data), &bytes_written)) {
            fprintf(stderr, "writing data to another file...\n");
            f_close(&acc_file);
          }
        }
#endif
      } else {
        printf("ERROR: Failed to open: %s\n", song_file_name);
      }
    } else {
      fprintf(stderr, "waiting for song name...\n");
    }

    f_close(&song_file);
  }
}

// Player task receives song data over Q_songdata to send it to the MP3 decoder
void mp3_player_task(void *p) {
  char bytes_512[512];

  while (1) {
    // xQueueReceive(Q_songdata, &bytes_512[0], portMAX_DELAY);
    // for (int i = 0; i < sizeof(bytes_512); i++) {
    //   while (!mp3_decoder_needs_data()) {
    //     vTaskDelay(1);
    //   }

    //   spi_send_to_mp3_decoder(bytes_512[i]);
    // }
    vTaskDelay(10000);
  }
}

void cli_led(void *p) {
  const port_pin_s *led_num = (port_pin_s *)(p);
  gpiox__set_as_output(*led_num);
  while (1) {
    gpiox__set_low(*led_num);
    vTaskDelay(300);
    gpiox__set_high(*led_num);
    vTaskDelay(300);
    gpiox__set_low(*led_num);
    vTaskDelay(300);
    gpiox__set_high(*led_num);
    vTaskDelay(1000);
  }
}

#endif

int main(void) {

#if 1
  Q_songname = xQueueCreate(1, sizeof(songname));
  Q_songdata = xQueueCreate(1, 512);
  sj2_cli__init();
  static port_pin_s cli_task_led = {2, 3};
  xTaskCreate(cli_led, "cli_led", (512U * 4) / sizeof(void *), &cli_task_led, 1, NULL);
  xTaskCreate(mp3_reader_task, "song_name", (1000 + (512U * 4) / sizeof(song_data_t)), NULL, 1, NULL);
  xTaskCreate(mp3_player_task, "play_song", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  vTaskStartScheduler();
#endif
}