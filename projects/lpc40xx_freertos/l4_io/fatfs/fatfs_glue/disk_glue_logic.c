/**
 * @file
 *
 * The FATFS connects to a 'disk' API defined at diskio.h
 * This is an implementation that provides the FATFS with its disk routines to read and write the disk.
 * You should not have to call these methods directly, instead opt to use high level f_open(), f_write() etc.
 */
#include "diskio.h"
#include "sd_card.h"
#include "ssp2_mutex.h"

/**
 * If you have multiple disk drives, add the accessor API here
 */
typedef enum {
  DRIVE__SD_CARD = 0,
} drive_e;

DSTATUS disk_initialize(BYTE drv) {
  DSTATUS status = RES_PARERR;

  spi2_mutex__acquire();
  {
    switch (drv) {
    case DRIVE__SD_CARD:
      status = sd_card__initialize();
      break;
    default:
      status = RES_PARERR;
      break;
    }
  }
  spi2_mutex__release();

  return status;
}

DSTATUS disk_status(BYTE drv) {
  DSTATUS status = RES_PARERR;

  // No mutex needed here
  {
    switch (drv) {
    case DRIVE__SD_CARD:
      status = sd_card__status();
      break;
    default:
      status = RES_PARERR;
      break;
    }
  }

  return status;
}

DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, UINT count) {
  DSTATUS status = RES_PARERR;

  spi2_mutex__acquire();
  {
    switch (drv) {
    case DRIVE__SD_CARD:
      status = sd_card__read(buff, sector, count);
      break;
    default:
      status = RES_PARERR;
      break;
    }
  }
  spi2_mutex__release();

  return status;
}

DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, UINT count) {
  DSTATUS status = RES_PARERR;

  spi2_mutex__acquire();
  {
    switch (drv) {
    case DRIVE__SD_CARD:
      status = sd_card__write(buff, sector, count);
      break;
    default:
      status = RES_PARERR;
      break;
    }
  }
  spi2_mutex__release();

  return status;
}

DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff) {
  DSTATUS status = RES_PARERR;

  spi2_mutex__acquire();
  {
    switch (drv) {
    case DRIVE__SD_CARD:
      status = sd_card__ioctl(ctrl, buff);
      break;
    default:
      status = RES_PARERR;
      break;
    }
  }
  spi2_mutex__release();

  return status;
}
