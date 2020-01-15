#pragma once

/**
 * @file
 *
 * This file connects the SD card logic to the platform independent board I/O
 */

#include "board_io.h"
#include "ssp2.h"
#include "sys_time.h"

// clang-format off
#define SD_CARD__SELECT()               board_io__sd_card_cs()
#define SD_CARD__DESELECT()             board_io__sd_card_ds()
#define SD_CARD__IS_PRESENT()           board_io__sd_card_is_present()

#define SD_CARD__SET_SLOW_SPEED()       ssp2__set_max_clock(  200)  /* Set slow SPI clock (100k-400k) */
#define SD_CARD__SET_FAST_SPEED()       ssp2__set_max_clock(24000)  /* Set fast SPI clock (depends on the CSD) */

#define SD_CARD__TRANSMIT_BYTE(byte)    ssp2__exchange_byte(byte)
#define SD_CARD__RECEIVE_BYTE()         ssp2__exchange_byte(0xff)

#define SD_CARD__PLATFORM_GET_MS()      ((uint32_t)sys_time__get_uptime_ms())

/**
 * @{
 * @name Read or Write larger blocks from/to the SD card
 * 
 * This can for example use the DMA
 */
#define SD_CARD__WRITE_BLOCK(output_block, number_of_bytes) \
    ssp2__dma_write_block(output_block, number_of_bytes)

#define SD_CARD__READ_BLOCK(input_block, number_of_bytes) \
    ssp2__dma_read_block(input_block, number_of_bytes)
/** @} */

// clang-format on