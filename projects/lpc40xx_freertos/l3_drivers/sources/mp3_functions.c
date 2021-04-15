#include "mp3_functions.h"
static port_pin_s adafruit_hardware_rst_pin = {0, 25};
static port_pin_s adafruit_cs_ds_pin = {0, 6};
static port_pin_s adafruit_dcs_dds_pin = {0, 16};

void ADA_DCS(void) { gpiox__set_low(adafruit_dcs_dds_pin); }
void ADA_DDS(void) { gpiox__set_high(adafruit_dcs_dds_pin); }

void adafruit_cs(void) { gpiox__set_low(adafruit_cs_ds_pin); }
void adafruit_ds(void) { gpiox__set_high(adafruit_cs_ds_pin); }

void adafruit_hw_rst(void) { gpiox__set_low(adafruit_hardware_rst_pin); }

void SCI_start_write(void) { spi0__mp3_exchange_byte(vs_write_op_code); }
void SCI_start_raed(void) { spi0__mp3_exchange_byte(vs_read_op_code); }

void SCI_select_system_mode(uint16_t system_mode) { SCI_write_16bits_reg_value(SCI_MODE, system_mode); }

void SCI_write_16bits_reg_value(SCI_REG_e reg_to_set, uint16_t value_to_write) {
  adafruit_cs();
  SCI_start_write();
  spi0__mp3_exchange_byte(reg_to_set);                   // send 8 bit register address
  spi0__mp3_exchange_byte((value_to_write >> 8) & 0xFF); // send bit 15 to bit 8 of data
  spi0__mp3_exchange_byte((value_to_write >> 0) & 0xFF); // send bit 8 to bit 0 of data
  adafruit_ds();
}

void SCI_read_16bits_reg_value(SCI_REG_e reg_to_set, uint16_t *reslt_container) { ; }

void spi_send_from_main_to_mp3_decoder(uint8_t one_byte_song_data) { spi0__mp3_exchange_byte(one_byte_song_data); }