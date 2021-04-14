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
#include "uart.h"
#include "uart_lab.h"

/******************************************************************************************/
/***************************************MP3 PROJECT****************************************/
#if 1
typedef char songname_t[16];
typedef char songname[32];

QueueHandle_t Q_songname;
QueueHandle_t Q_songdata;

app_cli_status_e cli__mp3_play(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                               app_cli__print_string_function cli_output) {
  // user_input_minus_command_name is actually a 'char *' pointer type
  // We tell the Queue to copy 32 bytes of songname from this location
  songname songname = {0};

  strncpy(songname, user_input_minus_command_name, sizeof(songname) - 1);
  printf("Sending input \"%s\" to queue\n", songname);
  xQueueSend(Q_songname, songname, portMAX_DELAY);
  return APP_CLI_STATUS__SUCCESS;
}

// Reader tasks receives song-name over Q_songname to start reading it
void mp3_reader_task(void *p) {
  songname name;
  char bytes_512[512];

  while (1) {
    xQueueReceive(Q_songname, &name, portMAX_DELAY);
    fprintf(stderr, "Received song to play: %d\n", name);

    // open_file();
    // while (!file.end()) {
    //   read_from_file(bytes_512);
    //   xQueueSend(Q_songdata, &bytes_512[0], portMAX_DELAY);
    // }
    // close_file();
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
#endif

int main(void) {

#if 1
  Q_songname = xQueueCreate(1, sizeof(songname));
  Q_songdata = xQueueCreate(1, 512);
  sj2_cli__init();
  xTaskCreate(mp3_reader_task, "song_name", (512U * 4) / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(mp3_player_task, "play_song", (512U * 4) / sizeof(void *), NULL, 1, NULL);
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
