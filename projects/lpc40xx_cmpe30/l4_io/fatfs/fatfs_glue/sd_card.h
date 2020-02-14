/**
 * @file
 *
 * This provides implementation for disk_glue_logic.c
 * Do not use this API directly; these are called by disk_glue_logic.c from the FATFS library.
 */
#pragma once

#include "diskio.h"

#define SD_CARD__READONLY 0
#define SD_CARD__USE_IOCTL 1

/// SD card initialization is performed automatically by FATFS through the diskio.h API
DSTATUS sd_card__initialize();

/// @returns status of card (if it has been initialized or not)
DSTATUS sd_card__status();

/// Reads one or more sectors
DRESULT sd_card__read(BYTE *buff, DWORD sector, BYTE count);

/// Writes one or more sectors
DRESULT sd_card__write(const BYTE *buff, DWORD sector, BYTE count);

/// Low level function used by FAT File System layer
DRESULT sd_card__ioctl(BYTE ctrl, void *buff);

/**
 * This can be schedule to periodically refresh the SD card insertion status
 * The read and write sector API will refresh the status upon entry to the function
 */
void sd_card__update_status(void);
