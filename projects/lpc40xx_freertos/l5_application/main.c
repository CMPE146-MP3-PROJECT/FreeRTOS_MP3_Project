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
#include "periodic_scheduler.h"
#include "pwm1.h"
#include "queue.h"
#include "sj2_cli.h"
#include "ssp2.h"
#include "task.h"
// #include "uart.h"
// #include "uart_lab.h"
#include "keypad.h"
#include "mp3_codec_spi.h"
#include "mp3_functions.h"

/******************************************************************************************/
/***************************************MP3 PROJECT****************************************/
#define SD_MP3_FILE_READ_WRITE_TEST 0
#define MP3_DECODER_STARTUP 1

#if MP3_DECODER_STARTUP

typedef char songname_t[16];
typedef char songname[32];
typedef uint8_t song_data_t[512];

QueueHandle_t Q_songname;
QueueHandle_t Q_songdata;
// QueueHandle_t Q_keypad_bottom;

static SemaphoreHandle_t song_playing_indication;
// SemaphoreHandle_t key_press_indication;

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

  // const *song_data_file_name = "bad.mp3";
  FIL song_file; /* [IN] File object */
  void *buff;    /* [OUT] Buffer to store read data */
  UINT btr;      /* [IN] Number of bytes to read */
  UINT *br;      /* [OUT] Number of bytes read */

  static UINT total_bytes_read = 0;
  FRESULT f_result; /* FatFs function common result code */
  while (1) {
    if (xQueueReceive(Q_songname, &song_file_name, 1000)) {
      fprintf(stderr, "\nReceived song to play: %s\n", song_file_name);
      // xSemaphoreGive(song_playing_indication);
      f_result = f_open(&song_file, song_file_name, FA_READ); // open_file();
      if (FR_OK == f_result) {
        while (f_eof(&song_file) == 0) { // while not reachh the end of the file
          f_read(&song_file, byte_512_song_data, sizeof(byte_512_song_data), &br);
          // fprintf(stderr, "song data: %s\n", byte_512_song_data);
          xQueueSend(Q_songdata, &byte_512_song_data, portMAX_DELAY);
        }
        f_close(&song_file);
      } else {
        printf("ERROR: Failed to open: %s\n", song_file_name);
      }
    } else {
      fprintf(stderr, "waiting for song name...\n");
    }
  }
}

// Player task receives song data over Q_songdata to send it to the MP3 decoder
void mp3_player_task(void *p) {
  song_data_t byte_512_song_data;
  UINT bytes_written = 0;
  FIL acc_file;
  int byte_send_per_time = 32;
  const *acc_data_file_name = "sensor_data_sd.txt";

  while (1) {
    if (xQueueReceive(Q_songdata, &byte_512_song_data, portMAX_DELAY)) {
      int byte_sent_number = 0;
      while (byte_sent_number < sizeof(byte_512_song_data)) {
        // while (mp3_decoder_needs_data() != 1) {
        //   printf("Bitstream FIFO not ready\n");
        //   vTaskDelay(portMAX_DELAY);
        // }
        while (mp3_decoder_needs_data() != 1) {
          // printf("Bitstream FIFO not ready\n");
          vTaskDelay(1);
        }
        // adafruit_cs();
        ADA_DCS();
        for (int j = byte_sent_number; j < (byte_sent_number + byte_send_per_time); j++) {
          // spi_send_from_main_to_mp3_decoder(byte_512_song_data[i]);
          spi0__mp3_exchange_byte(byte_512_song_data[j]);
          // puts("playing...");
        }
        byte_sent_number += byte_send_per_time;
        ADA_DDS();
        // adafruit_ds();
      }
      // vTaskDelay(100000);
#if SD_MP3_FILE_READ_WRITE_TEST
      if (FR_OK == f_open(&acc_file, acc_data_file_name, (FA_WRITE | FA_CREATE_ALWAYS))) {
        if (FR_OK == f_write(&acc_file, byte_512_song_data, sizeof(byte_512_song_data), &bytes_written)) {
          fprintf(stderr, "writing data to another file...\n");
          f_close(&acc_file);
        } else {
          fprintf(stderr, "fail to write...\n");
        }
      } else {
        fprintf(stderr, "fail to open: %c\n", acc_data_file_name);
      }
#endif
    }
  }
}

typedef enum {
  two = 0x2,
} x_two;

void write_data(uint16_t *ppp) { *ppp = 0x1234; }

void system_led(void *p) {
  // const port_pin_s *led_num = (port_pin_s *)(p);
  // uint16_t aaa;
  // write_data(&aaa);
  // fprintf(stderr, "aaa value: 0x%X", aaa);
  static port_pin_s system_led = {1, 18};
  pwm1__init_single_edge(1000);
  // gpio__construct_with_function(GPIO__PORT_1, 18, GPIO__FUNCTION_2); // PWM
  LPC_IOCON->P1_18 |= two;
  gpiox__set_as_output(system_led);
  uint8_t percent = 0;
  while (1) {
    pwm1__set_duty_cycle(PWM1__1_18, percent);

    if (++percent > 100) {
      percent = 0;
    }
    vTaskDelay(20);
  }
}

void system_init_count(void) {
  for (int i = 1; i <= 100; i++) {
    if (i == 1) {
      printf("system Initializing %3d%%", i);
    } else {
      printf("\b\b\b\b%3d%%", i);
    }
    fflush(stdout);
  }
}

void key_press_detect(void) {
  while (1) {
    read_keys();
  }
}
void key_press_ISR(void) {
  char key = 0;
  while (1) {
    if (xQueueReceive(Q_keypad_bottom, &key, 1000)) {
      fprintf(stderr, "key pressed is: %c\n", key);
      // switch (key){ //https://www.yiibai.com/book/article/820 <<-switch a char
      //   case 'B':
      //   break;
      // }
    } else {
      fprintf(stderr, "no key has been pressed\n");
    }
  }
}

int main(void) {
  /******************define system queues*********************/
  Q_songname = xQueueCreate(1, sizeof(songname));
  Q_songdata = xQueueCreate(1, 512);
  Q_keypad_bottom = xQueueCreate(1, sizeof(char));
  /***********************************************************/
  /****************define system semaphore********************/
  song_playing_indication = xSemaphoreCreateBinary();
  key_press_indication = xSemaphoreCreateBinary();
  /***********************************************************/
  /*****************system drivers startup********************/
  const uint32_t spi_clock_mhz = 1;
  sj2_cli__init();
  key_pins_init();
  spi0__mp3_init(spi_clock_mhz);
  // ssp2__lab_init(spi_clock_mhz);
  SCI_enable_DAC();
  system_init_count();
  // static port_pin_s cli_task_led = {2, 3};
  // SCI_select_system_mode(sdi_share_mode);
  // SCI_set_CLOCKF(0xC000); // SC_MULT[15:13] = 110 (XTALIx4.5); SC_ADD[12:11] = 00 (No modification)
  // SCI_set_volume(volume_quite);
  /***********************************************************/

  // xTaskCreate(system_led, "system_led", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(mp3_reader_task, "send_song", (2000 + sizeof(song_data_t)) / sizeof(void *), NULL, 2, NULL);
  xTaskCreate(mp3_player_task, "play_song", (2000 + sizeof(song_data_t)) / sizeof(void *), NULL, 3, NULL);
  xTaskCreate(key_press_detect, "key_detect", (512U * 4) / sizeof(void *), NULL, 3, NULL);
  xTaskCreate(key_press_ISR, "do_stuff", (512U * 4) / sizeof(void *), NULL, 4, NULL);
  vTaskStartScheduler();
}
#endif