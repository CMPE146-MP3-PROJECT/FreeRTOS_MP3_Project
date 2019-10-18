#pragma once

/**
 * @file
 *
 * Access to SPI-2 should go through mutex acquisition followed by the release.
 *
 * This ensures that while a task is reading/writing the SD card, another task
 * cannot simultaneously use the SPI bus for communication with another peripheral.
 *
 * Note that file access through the FATFS API is protected by this mutex at the
 * low level code, so you do not need to use this API while accessing FATFS API
 */

void spi2_mutex__acquire(void);
void spi2_mutex__release(void);
