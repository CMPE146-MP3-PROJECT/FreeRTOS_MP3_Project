#pragma once

#include "gpio.h"
#include "gpio_lab.h"
#include <stdint.h>
#include <stdlib.h>

// TODO: Study the Adesto flash 'Manufacturer and Device ID' section
typedef struct {
  uint8_t manufacturer_id;
  uint8_t device_id_1;
  uint8_t device_id_2;
  uint8_t extended_device_id;
} adesto_flash_id_s;

/**
 * This configures what DMA channels the SSP2 driver utilizes
 * for ssp2__dma_write_block() and ssp2__dma_read_block()
 */
#define SSP2__DMA_TX_CHANNEL 0
#define SSP2__DMA_RX_CHANNEL 1

/// Initialize the bus with the given maximum clock rate in Khz
void ssp2__initialize(uint32_t max_clock_khz);

/// After initialization, this allows you to change the bus clock speed
void ssp2__set_max_clock(uint32_t max_clock_khz);

/**
 * Exchange a single byte over the SPI bus
 * @returns the byte received while sending the byte_to_transmit
 */
uint8_t ssp2__exchange_byte(uint8_t byte_to_transmit);

/**
 * @{
 * @name    Exchanges larger blocks over the SPI bus
 * These are designed to be one-way transmission for the SPI for now
 */
void ssp2__dma_write_block(const unsigned char *output_block, size_t number_of_bytes);
void ssp2__dma_read_block(unsigned char *input_block, size_t number_of_bytes);
/** @} */
void ssp2__lab_init(uint32_t max_clock_mhz);

void ssp2__init_spi_pins(void);

uint8_t ssp2__lab_exchange_byte(uint8_t data_out);

adesto_flash_id_s adesto_read_signature(void);

void adesto_cs(void);

void adesto_ds(void);

void adesto_flash_send_address(uint32_t address);

void adesto_write_enable(void);

void adesto_write_disable(void);

uint8_t *adesto_read_arrary_address_input(uint32_t address);

void flash_erase_page(uint32_t address);
void flash_erase_chip(void);

void write_to_flash_8bitdata(uint32_t address, uint8_t data_in, int times);

uint8_t adesto_read_status();