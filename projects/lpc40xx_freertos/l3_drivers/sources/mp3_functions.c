#include "mp3_functions.h"
#include "task.h"

// static port_pin_s decoder_spi_ready_transfer_pin = {0, 22};
static port_pin_s adafruit_hardware_rst = {1, 14};
static port_pin_s adafruit_cs_ds = {0, 6};
static port_pin_s adafruit_dcs_dds = {0, 16};

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
  printf("\nDAC reset...\n");
}

void SCI_start_write(void) { spi0__mp3_exchange_byte(vs_write_op_code); }
void SCI_start_raed(void) { spi0__mp3_exchange_byte(vs_read_op_code); }

void SCI_select_system_mode(uint16_t SYSTEM_MODE) {
  SCI_write_16bits_reg_value(SCI_MODE, SYSTEM_MODE);
  puts("setting system mode...");
}
void SCI_set_CLOCKF(uint16_t CLOCKF_REG_VALUE) {
  SCI_write_16bits_reg_value(SCI_CLOCKF, CLOCKF_REG_VALUE);
  puts("setting clock mode...");
}
void SCI_set_volume(uint16_t SET_VOLUME) {
  SCI_write_16bits_reg_value(SCI_VOL, SET_VOLUME);
  puts("setting volume...");
}
void SCI_set_mono_stereo(uint16_t AU_DATA) {
  SCI_write_16bits_reg_value(SCI_AUDATA, AU_DATA);
  puts("sterro decoding mode...");
}

void SCI_enable_DAC(void) {

  adafruit_ds();
  ADA_DDS();
  adafruit_hw_rst();
  // vTaskDelay(100);

  SCI_select_system_mode(factory_EarSpeaker_low_mpde);
  SCI_select_system_mode(factory_EarSpeaker_low_mpde);
  SCI_set_CLOCKF(0xC000); // SC_MULT[15:13] = 110 (XTALIx4.5); SC_ADD[12:11] = 00 (No modification)
  SCI_set_volume(volume_quite);
  SCI_set_mono_stereo(stereo_decoding);

  SCI_16bits_data_s mode_red_data = SCI_read_16bits_reg_value(SCI_MODE);
  fprintf(stderr, "decoder mode_reg msb: 0x%X, lsb: 0x%X\n", mode_red_data.byte_one_msb, mode_red_data.byte_two_lsb);

  SCI_16bits_data_s volume_red_data = SCI_read_16bits_reg_value(SCI_VOL);
  fprintf(stderr, "volume_reg msb: 0x%X, lsb: 0x%X\n", volume_red_data.byte_one_msb, volume_red_data.byte_two_lsb);
  puts("decoder initialized");
}

void SCI_write_16bits_reg_value(uint8_t reg_to_set, uint16_t value_to_write) {
  adafruit_cs();
  SCI_start_write();
  spi0__mp3_exchange_byte(reg_to_set);                   // send 8 bit register address
  spi0__mp3_exchange_byte((value_to_write >> 8) & 0xFF); // send bit 15 to bit 8 of data
  spi0__mp3_exchange_byte((value_to_write >> 0) & 0xFF); // send bit 8 to bit 0 of data
  adafruit_ds();
}

SCI_16bits_data_s SCI_read_16bits_reg_value(uint8_t reg_to_read) {
  uint16_t dummy_byte_16bits = 0x1234;
  SCI_16bits_data_s reslt_container = {0};
  adafruit_cs();
  SCI_start_raed();
  spi0__mp3_exchange_byte(reg_to_read);
  reslt_container.byte_one_msb = spi0__mp3_exchange_byte((dummy_byte_16bits >> 8) & 0xFF);
  reslt_container.byte_two_lsb = spi0__mp3_exchange_byte((dummy_byte_16bits >> 0) & 0xFF);
  adafruit_ds();
  return reslt_container;
}

void spi_send_from_main_to_mp3_decoder(uint8_t one_byte_song_data) { spi0__mp3_exchange_byte(one_byte_song_data); }