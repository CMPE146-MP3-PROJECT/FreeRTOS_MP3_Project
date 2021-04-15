#pragma once
#include "mp3_codec_spi.h"
#include <stdbool.h>
#include <stdint.h>

/****************************************************************************/
/*****************SCI registers, prefix SCI_ @datasheet pg.37****************/
typedef enum {
  SCI_MODE = 0x00,        // Mode control, RW
  SCI_STATUS = 0x01,      // Status of VS1053b, RW
  SCI_BASS = 0x02,        // Built-in bass/treble control, RW
  SCI_CLOCKF = 0x03,      // Clock freq + multiplier, RW
  SCI_DECODE_TIME = 0x04, // Decode time in seconds, RW
  SCI_AUDATA = 0x05,      // Misc. audio data, RW
  SCI_WRAM = 0x06,        // RAM write/read, RW
  SCI_WRAMADDR = 0x07,    // Base address for RAM write/read, RW
  SCI_HDAT0 = 0x08,       // Stream header data 0, R only
  SCI_HDAT1 = 0x09,       // Stream header data 1, R only
  SCI_AIADDR = 0x0A,      // Start address of application, RW
  SCI_VOL = 0x0B,         // Volume control, RW
  SCI_AICTRL0 = 0x0C,     // Application control register 0, RW
  SCI_AICTRL1 = 0x0D,     // Application control register 1, RW
  SCI_AICTRL2 = 0x0E,     // Application control register 2, RW
  SCI_AICTRL3 = 0x0F,     // Application control register 3, RW
} SCI_REG_e;
/****************************************************************************/
/**************SCI Read/ Write operation, datasheet pg.20********************/
#define vs_write_op_code 0x2
#define vs_read_op_code 0x3
/****************************************************************************/
/****************System mode register value, datasheet pg.20*****************/
#define factory_sysytem_mode 0x4800 // SM_SDINEW set and LIN1 selected
#define sdi_share_mode 0x4C00       // SM_SDISHARE enableed, XDCS driven by XCS
/****************************************************************************/
typedef struct {
  uint8_t byte_one;
  uint8_t byte_two;
  uint8_t byte_three;
  uint8_t byte_four;
} SCI_32bits_data_s;

void adafruit_cs(void);
void adafruit_ds(void);

void ADA_DCS(void);
void ADA_DDS(void);

void adafruit_hw_rst(void);

void SCI_start_write(void);
void SCI_start_read(void);

void SCI_select_system_mode(uint16_t system_mode);

void SCI_write_16bits_reg_value(SCI_REG_e reg_to_set, uint16_t value_to_write);

void SCI_read_16bits_reg_value(SCI_REG_e reg_to_set, uint16_t *reslt_container);

void spi_send_from_main_to_mp3_decoder(uint8_t one_byte_song_data);
