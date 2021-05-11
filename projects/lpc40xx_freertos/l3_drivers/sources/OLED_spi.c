#include "OLED_spi.h"
#include "font_8x8_basic.h"

static port_pin_s OLED_DC_pin = {1, 25}; // active low
static port_pin_s EXT_OLED_DC_PIN = {1, 23};
static port_pin_s OLED_CS_pin = {1, 22}; // active low
static port_pin_s EXT_OLED_CS_PIN = {1, 29};
static port_pin_s OLED_SCK1_pin = {0, 7};
static port_pin_s OLED_MOSI1_pin = {0, 9};
static port_pin_s OLED_RST_pin = {2, 1};

static port_pin_s SCK1 = {0, 7};
static port_pin_s MOSI1 = {0, 9};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//                      P R I V A T E    F U N C T I O N S
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#define OLED_enable_horizontal_scroll 0

void spi1__pin_init(void) {

  uint8_t GPIO_pin_function_reset_mask = (7 << 0);

  gpio__construct_with_function(GPIO__PORT_1, 25, GPIO__FUNCITON_0_IO_PIN);
  gpiox__set_as_output(OLED_DC_pin);

  gpio__construct_with_function(GPIO__PORT_1, 23, GPIO__FUNCITON_0_IO_PIN);
  gpiox__set_as_output(EXT_OLED_DC_PIN);

  gpio__construct_with_function(GPIO__PORT_1, 22, GPIO__FUNCITON_0_IO_PIN);
  gpiox__set_as_output(OLED_CS_pin);

  gpio__construct_with_function(GPIO__PORT_1, 29, GPIO__FUNCITON_0_IO_PIN);
  gpiox__set_as_output(EXT_OLED_CS_PIN);

  gpio__construct_with_function(GPIO__PORT_2, 1, GPIO__FUNCITON_0_IO_PIN);
  gpiox__set_as_output(OLED_RST_pin);

  LPC_IOCON->P0_7 &= ~GPIO_pin_function_reset_mask;
  gpio__construct_with_function(GPIO__PORT_0, 7, GPIO__FUNCTION_2); // enable SSP1_SCK
  gpiox__set_as_output(SCK1);

  LPC_IOCON->P0_9 &= ~GPIO_pin_function_reset_mask;
  gpio__construct_with_function(GPIO__PORT_0, 9, GPIO__FUNCTION_2); // enable SSP1_MOSI
  gpiox__set_as_output(MOSI1);

  LPC_IOCON->P0_8 &= ~GPIO_pin_function_reset_mask;
  gpio__construct_with_function(GPIO__PORT_0, 8, GPIO__FUNCTION_2); // enable SSP1_MISO

  LPC_IOCON->P1_0 &= ~GPIO_pin_function_reset_mask;
  gpio__construct_with_function(GPIO__PORT_1, 0, GPIO__FUNCTION_4); // enable SSP2_SCK

  LPC_IOCON->P1_1 &= ~GPIO_pin_function_reset_mask;
  gpio__construct_with_function(GPIO__PORT_1, 1, GPIO__FUNCTION_4); // enable SSP2_MOSI

  LPC_IOCON->P1_4 &= ~GPIO_pin_function_reset_mask;
  gpio__construct_with_function(GPIO__PORT_1, 4, GPIO__FUNCTION_4); // enable SSP2_MISO

  // gpio__construct_with_function(GPIO__PORT_0, 7, GPIO__FUNCTION_2); // enable SSP1_SCK
  // gpio__construct_with_function(GPIO__PORT_0, 8, GPIO__FUNCTION_2); // enable SSP1_MISO
  // gpio__construct_with_function(GPIO__PORT_0, 9, GPIO__FUNCTION_2); // enable SSP1_MOSI
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//                      P U B L I C    F U N C T I O N S
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

void spi1__OLED_init(uint32_t max_spi_clock_mhz) {
  int power_on = 1;
  uint8_t spi_data_size_8_bit_mask = (7 << 0); //(7 << 0)
  uint8_t spi_controller_bit_mask = (1 << 1);  //(1 << 1); 0 = disable; 1 = enable
  //   uint8_t spi_MS_mode_bit_mask = (1 << 2);     //(1 << 2); 0 = master mode; 1 = slave mode;
  // this bit can only be written when SPI controller is disabled
  spi1__pin_init();                                   // enable spi0 pins
  Lpc_peripheral_power_control(LPC_PCSSP1, power_on); // power on SSP0
  LPC_SSP1->CR0 |= spi_data_size_8_bit_mask;

  const uint32_t cur_cpu_clk_mhz = UINT32_C(96) * 1000 * 1000;

  uint8_t devider_prescalar = (uint8_t)(96U / max_spi_clock_mhz);
  printf("  spi1_clk_div: 0x%X", devider_prescalar);
  LPC_SSP1->CPSR = devider_prescalar;

  LPC_SSP1->CR1 |= spi_controller_bit_mask; // enable spi1 controller

  OELD_transfer_data_byte(0xAA); // spi test
  OELD_transfer_data_byte(0xAA); // spi test
  OELD_transfer_data_byte(0xAA); // spi test
  OELD_transfer_data_byte(0xAA); // spi test

  OLED_function_init();
  char mp3[4] = {"[MP3]"};
  OLED_print_string(0, 5, 0, mp3, 5);
  OLED_print_string(1, 0, 0, "----------------", 16);
}

uint8_t spi1__OLED_exchange_byte(uint8_t data_out) {
  uint8_t spi_channel_busy_indication = (1 << 4); //(1 << 4); 1 = busy, 0 = free.
  LPC_SSP1->DR = (uint8_t)data_out;
  while (LPC_SSP1->SR & (1 << 4)) {
    ; // Wait until SSP is free
  }
  return (uint8_t)(LPC_SSP1->DR & 0xFF);
}

void OLED_RST(void) {
  gpiox__set_high(OLED_RST_pin);
  gpiox__set_low(OLED_RST_pin);
  gpiox__set_high(OLED_RST_pin);
}

void OLED_cs(void) {
  gpiox__set_low(OLED_CS_pin);
  gpiox__set_low(EXT_OLED_CS_PIN);
}

void OLED_ds(void) {
  gpiox__set_high(OLED_CS_pin);
  gpiox__set_high(EXT_OLED_CS_PIN);
}

void OLED_dc(bool status) {
  if (status) {
    gpiox__set_high(OLED_DC_pin);
    gpiox__set_high(EXT_OLED_DC_PIN);
  } else {
    gpiox__set_low(OLED_DC_pin);
    gpiox__set_low(EXT_OLED_DC_PIN);
  }
}

void OELD_transfer_data_byte(uint8_t data) {
  OLED_dc(1);
  OLED_cs();
  spi1__OLED_exchange_byte(data);
  OLED_ds();
}

void OELD_transfer_command_byte(uint8_t command) {
  OLED_dc(0);
  OLED_cs();
  spi1__OLED_exchange_byte(command);
  OLED_ds();
}

void OLED_Send_Char(uint8_t data) {
  uint8_t bit_map[8][8];
  uint8_t result[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      uint8_t temp = font8x8_basic[data][i];
      bit_map[j][i] = (temp >> j) & ~(0xFE);
    }
  }

  int result_index = 0;
  for (int i = 7; i >= 0; i--) {
    for (int j = 0; j < 8; j++) {
      result[result_index] |= bit_map[i][j] << j;
    }
    result_index++;
  }

  for (int i = 7; i >= 0; i--) {
    OELD_transfer_data_byte(result[i]);
  }
}

void OLED_print_string(uint8_t page, uint8_t upper_col, uint8_t lower_col, uint8_t *data, int length) {
  /// SSD1306 page32 addressing setting command table
  OELD_transfer_command_byte(0x20);
  OELD_transfer_command_byte(0x02); // choose Page Addressing Mode(Reset)

  OELD_transfer_command_byte(0x40);             // set display start line
  OELD_transfer_command_byte(0xB0 + page);      // 0xB0 setting page start address
  OELD_transfer_command_byte(0x00 + lower_col); // 0x00 - 0x0F set lower columu start adderss
  OELD_transfer_command_byte(0x10 + upper_col); // 0x10 - 0x1F set upper columu start adderss
  for (int i = 0; i < length; i++) {
    OLED_Send_Char(data[i]);
  }
}

void OLED_function_init(void) { // ER-OLED datasheet page.19

  OLED_RST();
  OELD_transfer_command_byte(0xAE); // set diaplay off

  OELD_transfer_command_byte(0xD5); // Display Clock Divide Ratio / Oscillator Frequency
  OELD_transfer_command_byte(0x10); // set CLK Fre, 0xF0 maximum frequency

  OELD_transfer_command_byte(0xA8);
  OELD_transfer_command_byte(0x3F); // set mulyiplex ratio

  OELD_transfer_command_byte(0xD3);
  OELD_transfer_command_byte(0x00); // set display offset

  OELD_transfer_command_byte(0x40); // Display Starting Line 40-7F

  OELD_transfer_command_byte(0x8D); // charge pump setting
  OELD_transfer_command_byte(0x14);

  OELD_transfer_command_byte(0xA1); // set segment re_map

  OELD_transfer_command_byte(0xC8); // Set COM output scan direction

  OELD_transfer_command_byte(0xDA); //
  OELD_transfer_command_byte(0x12); // Set COM Pins Hardware Configuration

  OELD_transfer_command_byte(0x81); // set contrast control
  OELD_transfer_command_byte(0x7F);

  OELD_transfer_command_byte(0xA4); // set entire display on/off

  OELD_transfer_command_byte(0xA6); // set normal/inverse display

  OLED_page_clear();

  OELD_transfer_command_byte(0XAF); // set diaplay on
}

void OLED_page_clear(void) {
  OELD_transfer_command_byte(0x20);
  OELD_transfer_command_byte(0x02); // choose Page Addressing Mode(Reset)

  for (int page = 0; page < 8; page++) {
    OELD_transfer_command_byte(0x40);
    OELD_transfer_command_byte(0xB0 + page); // 0xB0
    for (int higher_col = 0; higher_col < 8; higher_col++) {
      OELD_transfer_command_byte(0x10 + higher_col); // 0x10
      for (int lower_col = 0; lower_col < 16; lower_col++) {
        OELD_transfer_command_byte(0x00 + lower_col); // 0x00
        OELD_transfer_data_byte(0x00);
      }
    }
  }
}

void OLED_Horizontal_Scroll(uint8_t start_page, uint8_t end_page, uint8_t frame_frequency, uint8_t Right_Left,
                            bool auto_function) {
  /*
   start page and end page from 0 - 7
   time interval between each scroll step in terms of frame frequency:
                    0 - 5 frames,  1 - 64 frames,  2 - 128 frames,  3 - 256 frames,
                    4 - 3 frames,  5 - 4 frames,  6 - 25 frames,  7 - 2 frames,
                    lower the faster
  */

#if OLED_enable_horizontal_scroll

  OELD_transfer_command_byte(0x2F);
  OELD_transfer_command_byte(Right_Left);

  if (end_page <= start_page) {
    end_page = start_page;
  }
  OELD_transfer_command_byte(0x00); // A[7:0], Dummpy byte
  OELD_transfer_command_byte(start_page & 0x07);
  OELD_transfer_command_byte(frame_frequency & 0x07);
  OELD_transfer_command_byte(end_page & 0x07);
  OELD_transfer_command_byte(0x00); // Dummy Byte
  OELD_transfer_command_byte(0xFF); // Dummy Byte

  if (auto_function == true) {
    /// do nothing
  } else {
    OELD_transfer_command_byte(0x2E);
  }

#endif
}