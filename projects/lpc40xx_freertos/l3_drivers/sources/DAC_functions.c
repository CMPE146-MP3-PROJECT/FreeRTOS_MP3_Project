#include "DAC_functions.h"
#include "task.h"

/****************************************************************************/
/*****************SCI registers, prefix SCI_ @datasheet pg.37****************/
#define SCI_MODE 0x00        // Mode control, RW
#define SCI_STATUS 0x01      // Status of VS1053b, RW
#define SCI_BASS 0x02        // Built-in bass/treble control, RW
#define SCI_CLOCKF 0x03      // Clock freq + multiplier, RW
#define SCI_DECODE_TIME 0x04 // Decode time in seconds, RW
#define SCI_AUDATA 0x05      // Misc. audio data, RW
#define SCI_WRAM 0x06        // RAM write/read, RW
#define SCI_WRAMADDR 0x07    // Base address for RAM write/read, RW
#define SCI_HDAT0 0x08       // Stream header data 0, R only
#define SCI_HDAT1 0x09       // Stream header data 1, R only
#define SCI_AIADDR 0x0A      // Start address of application, RW
#define SCI_VOL 0x0B         // Volume control, RW
#define SCI_AICTRL0 0x0C     // Application control register 0, RW
#define SCI_AICTRL1 0x0D     // Application control register 1, RW
#define SCI_AICTRL2 0x0E     // Application control register 2, RW
#define SCI_AICTRL3 0x0F     // Application control register 3, RW
/****************************************************************************/
/**************SCI Read/ Write operation, datasheet pg.20********************/
#define vs_write_op_code 0x2
#define vs_read_op_code 0x3
/****************************************************************************/
/****************SYSTEM_MODE register value, datasheet pg.20*****************/
#define factory_sysytem_mode 0x4800 // SM_SDINEW set and LIN1 selected, mp3_function.h
#define sdi_share_mode 0x0C00       // SM_SDISHARE enableed, XDCS driven by XCS, mp3_function.h
#define factory_EarSpeaker_low_mpde 0x4810
/****************************************************************************/
/*****************SET_VOLUME register value, datasheet pg.47*****************/
#define analog_powerdowm 0xFFFF // mp3_function.h
#define volume_laud 0x0707      // L:7*(-0.5dB)/R:7*(-0.5dB)->L:-3.5dB/R:-3.5dB, mp3_function.h
#define volume_quite 0x3232
#define volume_normal 0x2020  // L:36*(-0.5dB)/R:36*(-0.5dB)->L:-18.0dB/R:-18.0dB,mp3_function.h
#define total_silence 0xFEFE  // mp3_function.h
#define maximum_volume 0x0000 // mp3_function.h
/****************************************************************************/
/******************AU_DATA register value, datasheet pg.43*******************/
#define stereo_decoding 0xAC45 // 44100 Hz stereo data reads
#define mono_decoding 0x2B10   // 11025 Hz mono data reads
/****************************************************************************/
/***************TREBLE_BASE register value, datasheet pg.41*******************/
#define bass_enhance 0x00F6
#define vocal_enhance 0x7A00
#define no_adjustment 0x0000
#define ready_adjustment 0x0108
/****************************************************************************/

// static port_pin_s decoder_spi_ready_transfer_pin = {0, 22};
static port_pin_s adafruit_hardware_rst = {1, 14};
static port_pin_s adafruit_cs_ds = {0, 6};
static port_pin_s adafruit_dcs_dds = {0, 16};

/**************for task control*****************/
TaskHandle_t song_reader_task;
TaskHandle_t song_player_task;
static bool play_pause_status = true;
static bool play_pause_status_OLED = true;
/***********************************************/
/*************for volume control****************/
static uint16_t volume_value = 0x2020;
static uint16_t volume_mask = 0x0202;
/***********************************************/
/***********for bass&treble control*************/
static uint16_t bass_treble_value = 0x0108;
static uint16_t bass_mask = 0x0010;
static uint16_t treble_mask = 0x1000;
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
  // puts("  no BASS adjustment...");
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
  SCI_set_bass_treble(ready_adjustment);

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
  ADA_DDS();
  adafruit_cs();

  SCI_start_raed();
  spi0__mp3_exchange_byte(reg_to_read);
  uint16_t msb = spi0__mp3_exchange_byte((dummy_byte_16bits >> 8) & 0xFF);
  uint8_t lsb = spi0__mp3_exchange_byte((dummy_byte_16bits >> 0) & 0xFF);

  adafruit_ds();
  ADA_DCS();
  // fprintf(stderr, "  msb: 0x%X lsb:0x%X\n", msb, lsb);
  msb = (msb << 8 | lsb);
  return msb;
}

void spi_send_from_main_to_mp3_decoder(uint8_t one_byte_song_data) { spi0__mp3_exchange_byte(one_byte_song_data); }

void DAC_volume_control(int plus_or_mins) {
  switch (plus_or_mins) {
  case 0:
    // DAC_volume_control(volume_down);
    if (volume_value == total_silence) {
      fprintf(stderr, "  Total silence\n");
    } else {
      volume_value = volume_value + volume_mask;
      SCI_set_volume(volume_value);
      fprintf(stderr, "  volume: L: -%ddB R: -%ddB\n", (volume_value >> 8), (volume_value & 0xFF));
      break;
    }

  case 1:
    // DAC_volume_control(volume_up);
    if (volume_value == 0x1010) {
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

void DAC_bass_control(int plus_or_mins) {
  uint8_t bass_value = ((bass_treble_value >> 4) & 0xF);
  switch (plus_or_mins) {
  case 0:
    if (bass_value == 0x0) {
      fprintf(stderr, "  bass: +0dB\n");
    } else {
      bass_treble_value = bass_treble_value - bass_mask;
      SCI_set_bass_treble(bass_treble_value);
      fprintf(stderr, "  bass: +%ddB\n", ((bass_treble_value >> 4) & 0xF));
    }
    break;

  case 1:
    if (bass_value == 0xF) {
      fprintf(stderr, "  bass: +15dB\n");
    } else {
      bass_treble_value = bass_treble_value + bass_mask;
      SCI_set_bass_treble(bass_treble_value);
      fprintf(stderr, "  bass: +%ddB\n", ((bass_treble_value >> 4) & 0xF));
    }
    break;
  }
}

void DAC_treble_control(int plus_or_mins) {
  uint8_t treble_value = ((bass_treble_value >> 12) & 0xF);
  switch (plus_or_mins) {
  case 0:
    // fprintf(stderr, "test value: %d", treble_value);

    if (treble_value > 0x0) {
      if (treble_value < 0x8) { //+7 +6 +5 +4 +3 +2 +1
        bass_treble_value = bass_treble_value - treble_mask;
        SCI_set_bass_treble(bass_treble_value);
        fprintf(stderr, "  treble: +%ddB\n", (((bass_treble_value >> 12) & 0xF)));
      } else if (treble_value == 0x8) { //+8
        fprintf(stderr, "  treble: -8dB\n");
      } else if (treble_value > 0x8) { //-1 -2 -3 -4 -5 -6 -7 -8
        bass_treble_value = bass_treble_value - treble_mask;
        SCI_set_bass_treble(bass_treble_value);
        fprintf(stderr, "  treble: %ddB\n", (((bass_treble_value >> 12) & 0xF) - 16));
      }
    } else if (treble_value == 0x0) { // 0
      bass_treble_value = bass_treble_value - treble_mask;
      SCI_set_bass_treble(bass_treble_value);
      fprintf(stderr, "  treble: %ddB\n", (((bass_treble_value >> 12) & 0xF) - 16));
    }
    break;

  case 1:
    if (treble_value > 0x0) {
      if (treble_value < 0x7) { //+1 +2 +3 +4 +5 +6
        bass_treble_value = bass_treble_value + treble_mask;
        SCI_set_bass_treble(bass_treble_value);
        fprintf(stderr, "  treble: +%ddB\n", (((bass_treble_value >> 12) & 0xF)));
      } else if (treble_value == 0x7) { //+7
        fprintf(stderr, "  treble: +7dB\n");
      } else if (treble_value > 7) {
        if (treble_value < 0xF) { // -7 -6 -5 -4 -3 -2
          bass_treble_value = bass_treble_value + treble_mask;
          SCI_set_bass_treble(bass_treble_value);
          fprintf(stderr, "  treble: %ddB\n", (((bass_treble_value >> 12) & 0xF) - 16));
        } else if (treble_value == 0xF) { //-1
          bass_treble_value = bass_treble_value + treble_mask;
          SCI_set_bass_treble(bass_treble_value);
          fprintf(stderr, "  treble: +0dB\n");
        }
      }
    } else if (treble_value == 0x0) { // +0
      bass_treble_value = bass_treble_value + treble_mask;
      SCI_set_bass_treble(bass_treble_value);
      fprintf(stderr, "  treble: +%ddB\n", ((bass_treble_value >> 12) & 0xF));
    }
    break;
  }
}

void DAC_play_pause(void) {
  song_reader_task = xTaskGetHandle("send_song");
  // song_player_task = xTaskGetHandle("play_song");
  if (play_pause_status) {
    play_pause_status = false;
    vTaskSuspend(song_reader_task);
    fprintf(stderr, "playback pause...\n");
  } else {
    play_pause_status = true;
    vTaskResume(song_reader_task);
    fprintf(stderr, "playback resume...\n");
  }
}

void OLED_play_pause(void) {

  if (play_pause_status_OLED) {
    play_pause_status_OLED = false;
    print_now_playing_page(0);
    print_play_pause_icon(0);
    OLED_Horizontal_Scroll(3, 3, 7, 0x27, 0);

  } else {
    play_pause_status_OLED = true;
    print_now_playing_page(1);
    print_play_pause_icon(1);
    OLED_Horizontal_Scroll(3, 3, 7, 0x27, 1);
  }
}