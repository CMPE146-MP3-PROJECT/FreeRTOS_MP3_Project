#pragma once
#include "gpio_lab.h"
#include "mp3_codec_spi.h"
#include "ssp2.h"
#include <stdbool.h>
#include <stdint.h>

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
#define no_adjustment 0x0002
/****************************************************************************/

// typedef struct {
//   uint8_t byte_one_msb;
//   uint8_t byte_two_lsb;
// } SCI_16bits_data_s;

// typedef struct {
//   uint8_t byte_one_msb;
//   uint8_t byte_two_lsb;
// } SCI_16bits_data_s;

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

// void DAC_play_pause(void);