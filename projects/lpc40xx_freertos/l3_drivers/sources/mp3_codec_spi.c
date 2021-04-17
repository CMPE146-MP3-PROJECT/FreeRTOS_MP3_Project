#include "mp3_codec_spi.h"
// decoder xcs  = P0_6
// decoder rst  = P0_25
// decoder dreq = P0_22
// decoder xdcs = P0_16
static port_pin_s decoder_spi_ready_transfer_pin = {0, 22};
static port_pin_s adafruit_hardware_rst_pin = {1, 14};
static port_pin_s adafruit_cs_ds_pin = {0, 6};
static port_pin_s adafruit_dcs_dds_pin = {0, 16};
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//                      P R I V A T E    F U N C T I O N S
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void spi0__pin_init(void) {
  uint8_t GPIO_pin_function_reset_mask = (7 << 0);

  gpio__construct_with_function(GPIO__PORT_0, 22, GPIO__FUNCITON_0_IO_PIN);
  gpiox__set_as_input(decoder_spi_ready_transfer_pin);

  gpio__construct_with_function(GPIO__PORT_1, 14, GPIO__FUNCITON_0_IO_PIN);
  gpiox__set_as_output(adafruit_hardware_rst_pin);

  gpio__construct_with_function(GPIO__PORT_0, 6, GPIO__FUNCITON_0_IO_PIN);
  gpiox__set_as_output(adafruit_cs_ds_pin);

  gpio__construct_with_function(GPIO__PORT_0, 16, GPIO__FUNCITON_0_IO_PIN);
  gpiox__set_as_output(adafruit_dcs_dds_pin);

  LPC_IOCON->P0_15 &= ~GPIO_pin_function_reset_mask;
  gpio__construct_with_function(GPIO__PORT_0, 15, GPIO__FUNCTION_2); // enable SSP0_SCK

  LPC_IOCON->P0_17 &= ~GPIO_pin_function_reset_mask;
  gpio__construct_with_function(GPIO__PORT_0, 17, GPIO__FUNCTION_2); // enable SSP0_MISO

  LPC_IOCON->P0_18 &= ~GPIO_pin_function_reset_mask;
  gpio__construct_with_function(GPIO__PORT_0, 18, GPIO__FUNCTION_2); // enable SSP0_MOSI
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//                      P U B L I C    F U N C T I O N S
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

void spi0__mp3_init(uint32_t max_spi_clock_mhz) {
  int power_on = 1;
  uint8_t spi_data_size_8_bit_mask = (7 << 0); //(7 << 0)
  uint8_t spi_controller_bit_mask = (1 << 1);  //(1 << 1); 0 = disable; 1 = enable
  uint8_t spi_MS_mode_bit_mask = (1 << 2);     //(1 << 2); 0 = master mode; 1 = slave mode;
                                               // this bit can only be written when SPI controller is disabled
  spi0__pin_init();                            // enable spi0 pins
  // a) Power on Peripheral
  Lpc_peripheral_power_control(LPC_PCSSP0, power_on); // power on SSP0
  // b) Setup control registers CR0 and CR1
  LPC_SSP0->CR0 |= spi_data_size_8_bit_mask;
  // LPC_SSP0->CR1 &= ~spi_controller_bit_mask; // disable spi0 controller
  // LPC_SSP0->CR1 &= ~spi_MS_mode_bit_mask;    // spi0 master mode
  LPC_SSP0->CR1 |= spi_controller_bit_mask; // enable spi0 controller
  // c) Setup prescalar register to be <= max_clock_mhz
  const uint32_t cur_cpu_clk_mhz = UINT32_C(96) * 1000 * 1000;
  uint8_t devider_prescalar = 2;
  while (max_spi_clock_mhz < (cur_cpu_clk_mhz / devider_prescalar) && devider_prescalar <= 254) {
    devider_prescalar += 2;
  }
  LPC_SSP2->CPSR = devider_prescalar;
}

uint8_t spi0__mp3_exchange_byte(uint8_t data_out) {
  // if (LPC_SSP2->SR & (1 << 1)) { // test if Transmit FIFO is Full
  uint8_t spi_channel_busy_indication = (1 << 4); //(1 << 4); 1 = busy, 0 = free.
  LPC_SSP0->DR = data_out;                        // write data to DR ( 1 byte data + 1 byte endFillByte)
  while (LPC_SSP0->SR & spi_channel_busy_indication) {
    ; // Wait until SSP is free
  }
  return (uint8_t)(LPC_SSP0->DR & 0xFF);
  //} else {
  // puts("cannot wirte to DR, because Transmit FIFO is not empty");
  //}
}

int mp3_decoder_needs_data() { return gpiox__get_level(decoder_spi_ready_transfer_pin); }
