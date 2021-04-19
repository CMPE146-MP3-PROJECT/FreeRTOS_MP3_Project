#include "DAC_functions.h"
#include "task.h"

// static port_pin_s decoder_spi_ready_transfer_pin = {0, 22};
static port_pin_s adafruit_hardware_rst = {1, 14};
static port_pin_s adafruit_cs_ds = {0, 6};
static port_pin_s adafruit_dcs_dds = {0, 16};

/**************for task control*****************/
// TaskHandle_t song_reader_task;
// TaskHandle_t song_player_task;
// static bool play_pause_status = true;
/***********************************************/
/*************for volume control****************/
static uint16_t volume_value = 0x2020;
static uint16_t volume_mask = 0x0202;
/***********************************************/

void ADA_DCS(void) { gpiox__set_low(adafruit_dcs_dds); }
void ADA_DDS(void) { gpiox__set_high(adafruit_dcs_dds); }

void adafruit_cs(void) { gpiox__set_low(adafruit_cs_ds); }
void adafruit_ds(void) { gpiox__set_high(adafruit_cs_ds); }

void adafruit_hw_rst(void) {
  // gpiox__set_low(adafruit_hardware_rst);
  // vTaskDelay(150);
  // puts("1...");
  gpiox__set_high(adafruit_hardware_rst);
  // vTaskDelay(50);
  printf("\n  DAC reset...\n");
}

void SCI_start_write(void) { spi0__mp3_exchange_byte(vs_write_op_code); }
void SCI_start_raed(void) { spi0__mp3_exchange_byte(vs_read_op_code); }

void SCI_select_system_mode(uint16_t SYSTEM_MODE) {
  SCI_write_16bits_reg_value(SCI_MODE, SYSTEM_MODE);
  puts("  setting system mode...");
}
void SCI_set_CLOCKF(uint16_t CLOCKF_REG_VALUE) {
  SCI_write_16bits_reg_value(SCI_CLOCKF, CLOCKF_REG_VALUE);
  puts("  setting clock mode...");
}
void SCI_set_volume(uint16_t SET_VOLUME) {
  SCI_write_16bits_reg_value(SCI_VOL, SET_VOLUME);
  // puts("setting volume...");
}

void SCI_set_mono_stereo(uint16_t AU_DATA) {
  SCI_write_16bits_reg_value(SCI_AUDATA, AU_DATA);
  puts("  sterro decoding mode...");
}

void SCI_set_bass_treble(uint16_t TREBLE_BASE) {
  SCI_write_16bits_reg_value(SCI_BASS, TREBLE_BASE);
  puts("  no BASS adjustment...");
}

void SCI_enable_DAC(void) {

  adafruit_ds();
  ADA_DDS();
  adafruit_hw_rst();
  // vTaskDelay(100);

  SCI_select_system_mode(factory_sysytem_mode);
  SCI_select_system_mode(factory_sysytem_mode);
  SCI_set_CLOCKF(0xC000); // SC_MULT[15:13] = 110 (XTALIx4.5); SC_ADD[12:11] = 00 (No modification)
  SCI_set_volume(volume_normal);
  SCI_set_mono_stereo(stereo_decoding);
  SCI_set_bass_treble(no_adjustment);

  uint16_t mode_reg_data = SCI_read_16bits_reg_value(SCI_MODE);
  fprintf(stderr, "  mode_reg: 0x%X\n", mode_reg_data);

  SCI_read_volume();
  puts("  decoder initialized");
}

void SCI_write_16bits_reg_value(uint8_t reg_to_set, uint16_t value_to_write) {
  // while (mp3_decoder_needs_data() != 1)
  //   ;
  ADA_DDS();
  adafruit_cs();

  SCI_start_write();
  spi0__mp3_exchange_byte(reg_to_set);                   // send 8 bit register address
  spi0__mp3_exchange_byte((value_to_write >> 8) & 0xFF); // send bit 15 to bit 8 of data
  spi0__mp3_exchange_byte((value_to_write >> 0) & 0xFF); // send bit 8 to bit 0 of data

  // while (mp3_decoder_needs_data() != 1)
  //   ;
  adafruit_ds();
  ADA_DCS();
}

uint16_t SCI_read_16bits_reg_value(uint8_t reg_to_read) {
  uint16_t dummy_byte_16bits = 0x1234;
  // uint16_t reslt_container = 0x0000;
  // static uint16_t msb = 0x00;
  // static uint8_t lsb = 0x00;
  adafruit_cs();
  SCI_start_raed();
  spi0__mp3_exchange_byte(reg_to_read);
  uint16_t msb = spi0__mp3_exchange_byte((dummy_byte_16bits >> 8) & 0xFF);
  uint8_t lsb = spi0__mp3_exchange_byte((dummy_byte_16bits >> 0) & 0xFF);
  adafruit_ds();
  // fprintf(stderr, "  msb: 0x%X lsb:0x%X\n", msb, lsb);
  msb = (msb << 8 | lsb);
  return msb;
}

void spi_send_from_main_to_mp3_decoder(uint8_t one_byte_song_data) { spi0__mp3_exchange_byte(one_byte_song_data); }

void DAC_volume_control(int plus_or_mins) {
  switch (plus_or_mins) {
  case 0:
    // DAC_volume_control(volume_down);
    volume_value = volume_value + volume_mask;
    SCI_set_volume(volume_value);
    fprintf(stderr, "  volume: L: -%ddB R: -%ddB\n", (volume_value >> 8), (volume_value & 0xFF));
    break;

  case 1:
    // DAC_volume_control(volume_up);
    if (volume_value == 0x1212) {
      fprintf(stderr, "  warning: too laud volume may damage your ear!\n");
    } else {
      volume_value = volume_value - volume_mask;
      SCI_set_volume(volume_value);
      fprintf(stderr, "  volume: L: -%ddB R: -%ddB\n", (volume_value >> 8), (volume_value & 0xFF));
    }
    break;
  }
}

void SCI_read_volume(void) {
  uint16_t volume_data = SCI_read_16bits_reg_value(SCI_VOL);
  // fprintf(stderr, "  volume: L:-%ddB R:-%ddB\n", (volume_data >> 8), (volume_data & 0xFF));
  fprintf(stderr, "  volume: 0x%X\n", volume_data);
}

// void DAC_play_pause(void) {
//   // song_reader_task = xTaskGetHandle("send_song");
//   song_player_task = xTaskGetHandle("play_song");
//   if (play_pause_status) {
//     play_pause_status = false;
//     vTaskSuspend(song_reader_task);
//     fprintf(stderr, "playback pause...\n");
//   } else {
//     play_pause_status = true;
//     vTaskResume(song_reader_task);
//     fprintf(stderr, "playback resume...\n");
//   }
// }