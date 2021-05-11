#pragma once
#include "OLED.h"
#include "OLED_spi.h"
#include "gpio_lab.h"
#include "mp3_codec_spi.h"
#include "ssp2.h"
#include <stdbool.h>
#include <stdint.h>

void adafruit_cs(void);
void adafruit_ds(void);

void ADA_DCS(void);
void ADA_DDS(void);

void adafruit_hw_rst(void);

void SCI_start_write(void);
void SCI_start_raed(void);

void SCI_select_system_mode(uint16_t SYSTEM_MODE);
void SCI_set_CLOCKF(uint16_t CLOCKF_REG_VALUE);
void SCI_set_volume(uint16_t SET_VOLUME);
void SCI_set_mono_stereo(uint16_t AU_DATA);
void SCI_set_bass_treble(uint16_t TREBLE_BASE);

void SCI_enable_DAC(void);

void SCI_write_16bits_reg_value(uint8_t reg_to_set, uint16_t value_to_write);
uint16_t SCI_read_16bits_reg_value(uint8_t reg_to_read);

void spi_send_from_main_to_mp3_decoder(uint8_t one_byte_song_data);

void DAC_volume_control(int plus_or_mins);

void SCI_read_volume(void);

void DAC_play_pause(void);

void DAC_bass_control(int plus_or_mins);

void DAC_treble_control(int plus_or_mins);

void OLED_play_pause(void);