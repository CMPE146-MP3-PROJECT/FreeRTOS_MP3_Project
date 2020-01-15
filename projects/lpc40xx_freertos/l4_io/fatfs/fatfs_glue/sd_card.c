#include "sd_card.h"

#include "sd_card_defines.h"

/* Definitions for MMC/SDC command */
#define CMD0 (0x40 + 0)    /* GO_IDLE_STATE */
#define CMD1 (0x40 + 1)    /* SEND_OP_COND (MMC) */
#define ACMD41 (0xC0 + 41) /* SEND_OP_COND (SDC) */
#define CMD8 (0x40 + 8)    /* SEND_IF_COND */
#define CMD9 (0x40 + 9)    /* SEND_CSD */
#define CMD10 (0x40 + 10)  /* SEND_CID */
#define CMD12 (0x40 + 12)  /* STOP_TRANSMISSION */
#define ACMD13 (0xC0 + 13) /* SD_STATUS (SDC) */
#define CMD16 (0x40 + 16)  /* SET_BLOCKLEN */
#define CMD17 (0x40 + 17)  /* READ_SINGLE_BLOCK */
#define CMD18 (0x40 + 18)  /* READ_MULTIPLE_BLOCK */
#define CMD23 (0x40 + 23)  /* SET_BLOCK_COUNT (MMC) */
#define ACMD23 (0xC0 + 23) /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24 (0x40 + 24)  /* WRITE_BLOCK */
#define CMD25 (0x40 + 25)  /* WRITE_MULTIPLE_BLOCK */
#define CMD55 (0x40 + 55)  /* APP_CMD */
#define CMD58 (0x40 + 58)  /* READ_OCR */

/* MMC/SDC command */
#define MMC_GET_TYPE 10
#define MMC_GET_CSD 11
#define MMC_GET_CID 12
#define MMC_GET_OCR 13
#define MMC_GET_SDSTAT 14
/* ATA/CF command */
#define ATA_GET_REV 20
#define ATA_GET_MODEL 21
#define ATA_GET_SN 22

/* Card type flags (CardType) */
#define CT_MMC 0x01
#define CT_SD1 0x02
#define CT_SD2 0x04
#define CT_SDC (CT_SD1 | CT_SD2)
#define CT_BLOCK 0x08

#define DEBUG_SD_CARD 0          // Set to 1 to printf debug data
#define OPTIMIZE_SSP_SPI_WRITE 0 // Set to 1 to use SD_CARD__WRITE_BLOCK()
#define OPTIMIZE_SSP_SPI_READ 1  // Set to 1 to use SD_CARD__READ_BLOCK()

#if DEBUG_SD_CARD
#include <stdio.h>
#define SD_CARD__DEBUG_PRINTF(f_, ...) printf((f_), ##__VA_ARGS__)
#else
#define SD_CARD__DEBUG_PRINTF(f_, ...) /* NOOP */
#endif

static volatile DSTATUS g_disk_status = STA_NOINIT; ///< Disk status
static BYTE g_card_type;                            ///< Card type flags

static BYTE sd_card__wait_for_ready(void) {
  BYTE res;

  /* Wait for ready in timeout of 500ms */
  const uint32_t timeout = SD_CARD__PLATFORM_GET_MS() + 500;
  SD_CARD__RECEIVE_BYTE();

  do {
    res = SD_CARD__RECEIVE_BYTE();
  } while ((res != 0xFF) && SD_CARD__PLATFORM_GET_MS() < timeout);

  return res;
}

static void sd_card__power_on(void) {
  // Power on the SD-Card Socket if hardware allows
}

static void sd_card__power_off(void) {
  SD_CARD__SELECT();
  sd_card__wait_for_ready();
  SD_CARD__DESELECT();

  // Power off the SD-Card Socket if hardware allows
  g_disk_status |= STA_NOINIT; // Set STA_NOINIT
}

static int sd_card__read_datablock(BYTE *buff, /* Data buffer to store received data */
                                   UINT btr    /* Byte count (must be multiple of 4) */
) {
  BYTE token;
  const uint32_t time_now = SD_CARD__PLATFORM_GET_MS();
  const uint32_t timeout = time_now + 100;
  int count = 0;

  do { /* Wait for data packet in timeout of 100ms */
    token = SD_CARD__RECEIVE_BYTE();
    ++count;

    if (SD_CARD__PLATFORM_GET_MS() > timeout) {
      break;
    }
  } while (token == 0xFF);

  if (token != 0xFE) {
    SD_CARD__DEBUG_PRINTF("ERROR: token not 0xFE after %i bytes (started %lu, now %lu)\n", count, time_now, SD_CARD__PLATFORM_GET_MS());
    return 0; /* If not valid data token, return with error */
  }

  /**
   * If it's worth doing DMA, then do it:
   */
  if (OPTIMIZE_SSP_SPI_READ && btr >= 16) {
    SD_CARD__READ_BLOCK(buff, 512);
    buff += 512;
  } else {
    do {
      *buff++ = SD_CARD__RECEIVE_BYTE();
      *buff++ = SD_CARD__RECEIVE_BYTE();
      *buff++ = SD_CARD__RECEIVE_BYTE();
      *buff++ = SD_CARD__RECEIVE_BYTE();
    } while (btr -= 4);
  }

  SD_CARD__RECEIVE_BYTE(); /* Discard CRC */
  SD_CARD__RECEIVE_BYTE();

  return 1; /* Return with success */
}

#if SD_CARD__READONLY == 0
static int sd_card__transmit_datablock(const BYTE *buff, /* 512 byte data block to be transmitted */
                                       BYTE token        /* Data/Stop token */
) {
  BYTE resp;

  if (sd_card__wait_for_ready() != 0xFF) {
    return 0;
  }

  SD_CARD__TRANSMIT_BYTE(token);
  /* Xmit data token */
  if (token != 0xFD) { /* Is data token */
#if OPTIMIZE_SSP_SPI_WRITE
    SD_CARD__WRITE_BLOCK(buff, 512);
#else
    unsigned char wc = 0;
    do { /* Xmit the 512 byte data block to MMC */
      SD_CARD__TRANSMIT_BYTE(*buff++);
      SD_CARD__TRANSMIT_BYTE(*buff++);
    } while (--wc);
#endif

    SD_CARD__TRANSMIT_BYTE(0xFF);
    /* CRC (Dummy) */
    SD_CARD__TRANSMIT_BYTE(0xFF);
    resp = SD_CARD__RECEIVE_BYTE(); /* Reveive data response */
    if ((resp & 0x1F) != 0x05) {      /* If not accepted, return with error */
      return 0;
    }
  }

  return 1;
}
#endif /* SD_CARD__READONLY */

static BYTE sd_card__send_command(BYTE cmd, /* Command byte */
                                  DWORD arg /* Argument */
) {
  BYTE n, res;

  if (cmd & 0x80) { /* ACMD<n> is the command sequense of CMD55-CMD<n> */
    cmd &= 0x7F;
    res = sd_card__send_command(CMD55, 0);
    if (res > 1) {
      SD_CARD__DEBUG_PRINTF("ERROR: While sending CMD55\n");
      return res;
    }
  }

  /* Select the card and wait for ready */
  SD_CARD__DESELECT();
  SD_CARD__SELECT(); // Wait for card ready

  if (sd_card__wait_for_ready() != 0xFF) {
    SD_CARD__DEBUG_PRINTF("ERROR: sd_card__wait_for_ready() did not return 0xFF\n");
    return 0xFF;
  }

  if (cmd == CMD0) {
    n = 0x95; /* Valid CRC for CMD0(0) */
  }
  else if (cmd == CMD8) {
    n = 0x87; /* Valid CRC for CMD8(0x1AA) */
  } else {
    n = 0x01; /* Dummy CRC + Stop */
  }

  /* Start + Command index + 32-bit arguments */
  const uint8_t bytes_to_transmit[] = {cmd, (BYTE)(arg >> 24), (BYTE)(arg >> 16), (BYTE)(arg >> 8), (BYTE)(arg >> 0), n};
  ssp2__dma_write_block(bytes_to_transmit, sizeof(bytes_to_transmit));

  /* Receive command response */
  if (cmd == CMD12) {
    SD_CARD__RECEIVE_BYTE(); /* Skip a stuff byte when stop reading */
  }
  
  n = 10; /* Wait for a valid response in timeout of 10 attempts */
  do {
    res = SD_CARD__RECEIVE_BYTE();
  } while ((res & 0x80) && --n);

  if (n == 0) {
    SD_CARD__DEBUG_PRINTF("Timeout during card read\n");
  }

  return res; /* Return with the response value */
}

DSTATUS sd_card__initialize() {
  BYTE n, cmd, ty, ocr[4];

  sd_card__update_status();

  if (g_disk_status & STA_NODISK) {
    return g_disk_status; /* No card in the socket */
  }

  sd_card__power_on(); /* Force socket power on */
  SD_CARD__SET_SLOW_SPEED();
  for (n = 10; n; n--) {
    SD_CARD__RECEIVE_BYTE(); /* 80 dummy clocks */
  }

  SD_CARD__SELECT();
  ty = 0;

  if (sd_card__send_command(CMD0, 0) == 1) { /* Enter Idle state */
    const uint32_t timeout = SD_CARD__PLATFORM_GET_MS() + 1000;
    if (sd_card__send_command(CMD8, 0x1AA) == 1) { /* SDHC */\
      SD_CARD__DEBUG_PRINTF("CMD8 succeeded...\n");

      for (n = 0; n < 4; n++)
        ocr[n] = SD_CARD__RECEIVE_BYTE();     /* Get trailing return value of R7 resp */
      if (ocr[2] == 0x01 && ocr[3] == 0xAA) { /* The card can work at vdd range of 2.7-3.6V */
        SD_CARD__DEBUG_PRINTF("SD-HC Card detected!\n");

        while (SD_CARD__PLATFORM_GET_MS() < timeout && sd_card__send_command(ACMD41, 1UL << 30)) {
          ; /* Wait for leaving idle state (ACMD41 with HCS bit) */
        }

        if (SD_CARD__PLATFORM_GET_MS() < timeout &&
            sd_card__send_command(CMD58, 0) == 0) { /* Check CCS bit in the OCR */
          for (n = 0; n < 4; n++) {
            ocr[n] = SD_CARD__RECEIVE_BYTE();
          }
          ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
        } else {
          SD_CARD__DEBUG_PRINTF("CMD58 FAILED!\n");
        }
      }
    } else { /* SDSC or MMC */
      SD_CARD__DEBUG_PRINTF("Card is not SD-HC!\n");

      if (sd_card__send_command(ACMD41, 0) <= 1) {
        ty = CT_SD1;
        cmd = ACMD41; /* SDSC */
      } else {
        ty = CT_MMC;
        cmd = CMD1; /* MMC */
      }
      while (SD_CARD__PLATFORM_GET_MS() < timeout && sd_card__send_command(cmd, 0)) {
        ; /* Wait for leaving idle state */
      }

      if ((SD_CARD__PLATFORM_GET_MS() < timeout) ||
          sd_card__send_command(CMD16, 512) != 0) /* Set R/W block length to 512 */
      {
        SD_CARD__DEBUG_PRINTF("Could not set block length to 512\n");
        ty = 0;
      }
    }
  } else {
    SD_CARD__DEBUG_PRINTF("CMD0 did not respond...\n");
  }

  g_card_type = ty;
  SD_CARD__DESELECT();

  if (ty) {                       /* Initialization succeded */
    g_disk_status &= ~STA_NOINIT; /* Clear STA_NOINIT */
    SD_CARD__SET_FAST_SPEED();
  } else { /* Initialization failed */
    sd_card__power_off();
  }

  return g_disk_status;
}

DSTATUS sd_card__status() {
  sd_card__update_status();
  return g_disk_status;
}

DRESULT sd_card__read(BYTE *buff,   /* Pointer to the data buffer to store read data */
                      DWORD sector, /* Start sector number (LBA) */
                      BYTE count    /* Sector count (1..255) */
) {
  sd_card__update_status();

  if (!count) {
    SD_CARD__DEBUG_PRINTF("ERROR: sector count");
    return RES_PARERR;
  }
  if (g_disk_status & STA_NOINIT) {
    SD_CARD__DEBUG_PRINTF("ERROR: disk init");
    return RES_NOTRDY;
  }

  SD_CARD__SELECT();
  if (!(g_card_type & CT_BLOCK)) {
    sector *= 512; /* Convert to byte address if needed */
  }

  if (count == 1) {                                 /* Single block read */
    if ((sd_card__send_command(CMD17, sector) == 0)) {
      if (sd_card__read_datablock(buff, 512)) {
        count = 0;
      } else {
        SD_CARD__DEBUG_PRINTF("ERROR during sd_card__read_datablock()");
      }
    } else {
      SD_CARD__DEBUG_PRINTF("ERROR during sd_card__send_command()");
    }
  } else {                                           /* Multiple block read */
    if (sd_card__send_command(CMD18, sector) == 0) { /* READ_MULTIPLE_BLOCK */
      do {
        if (!sd_card__read_datablock(buff, 512))
          break;
        buff += 512;
      } while (--count);
      sd_card__send_command(CMD12, 0); /* STOP_TRANSMISSION */
    }
  }
  SD_CARD__DESELECT();

  return count ? RES_ERROR : RES_OK;
}

#if SD_CARD__READONLY == 0
DRESULT sd_card__write(const BYTE *buff, /* Pointer to the data to be written */
                       DWORD sector,     /* Start sector number (LBA) */
                       BYTE count        /* Sector count (1..255) */
) {
  sd_card__update_status();

  if (!count) {
    return RES_PARERR;
  }
  if (g_disk_status & STA_NOINIT) {
    return RES_NOTRDY;
  }
  if (g_disk_status & STA_PROTECT) {
    return RES_WRPRT;
  }

  SD_CARD__SELECT();
  if (!(g_card_type & CT_BLOCK)) {
    sector *= 512; /* Convert to byte address if needed */
  }

  if (count == 1) { /* Single block write */
    if ((sd_card__send_command(CMD24, sector) == 0) && sd_card__transmit_datablock(buff, 0xFE)) {
      count = 0;
    }
  } else {
    if (g_card_type & CT_SDC) {
      sd_card__send_command(ACMD23, count);
    }

    if (sd_card__send_command(CMD25, sector) == 0) { /* WRITE_MULTIPLE_BLOCK */
      do {
        if (!sd_card__transmit_datablock(buff, 0xFC)) {
          break;
        }
        buff += 512;
      } while (--count);
      if (!sd_card__transmit_datablock(0, 0xFD)) {/* STOP_TRAN token */
        count = 1;
      }
    }
  }
  SD_CARD__DESELECT();

  return count ? RES_ERROR : RES_OK;
}
#endif /* SD_CARD__READONLY == 0 */

#if SD_CARD__USE_IOCTL != 0
DRESULT sd_card__ioctl(BYTE ctrl, /* Control code */
                       void *buff /* Buffer to send/receive control data */
) {
  BYTE n, csd[16], *ptr = (BYTE *)buff;
  WORD csize;

  sd_card__update_status();
  DRESULT res = RES_ERROR;

  if (ctrl == CTRL_POWER) {
    switch (*ptr) {
    case 0: /* Sub control code == 0 (sd_card__power_off) */
      if (SD_CARD__IS_PRESENT()) {
        sd_card__power_off();
      }
      res = RES_OK;
      break;

    case 1:                /* Sub control code == 1 (sd_card__power_on) */
      sd_card__power_on();
      res = RES_OK;
      break;

    case 2: /* Sub control code == 2 (POWER_GET) */
      *(ptr + 1) = (BYTE)SD_CARD__IS_PRESENT();
      res = RES_OK;
      break;

    default:
      res = RES_PARERR;
      break;
    }
  } else {
    if (g_disk_status & STA_NOINIT) {
      return RES_NOTRDY;
    }

    switch (ctrl) {
    case CTRL_SYNC: /* Make sure that no pending write process */
      SD_CARD__SELECT();
      if (sd_card__wait_for_ready() == 0xFF) {
        res = RES_OK;
      }
      break;

    case GET_SECTOR_COUNT: /* Get number of sectors on the disk (DWORD) */
      if ((sd_card__send_command(CMD9, 0) == 0) && sd_card__read_datablock(csd, 16)) {
        if ((csd[0] >> 6) == 1) { /* SDC ver 2.00 */
          csize = csd[9] + ((WORD)csd[8] << 8) + 1;
          *(DWORD *)buff = (DWORD)csize << 10;
        } else { /* SDC ver 1.XX or MMC*/
          n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
          csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
          *(DWORD *)buff = (DWORD)csize << (n - 9);
        }
        res = RES_OK;
      }
      break;

    case GET_SECTOR_SIZE: /* Get R/W sector size (WORD) */
      *(WORD *)buff = 512;
      res = RES_OK;
      break;

    case GET_BLOCK_SIZE:                             /* Get erase block size in unit of sector (DWORD) */
      if (g_card_type & CT_SD2) {                    /* SDC ver 2.00 */
        if (sd_card__send_command(ACMD13, 0) == 0) { /* Read SD status */
          SD_CARD__RECEIVE_BYTE();
          if (sd_card__read_datablock(csd, 16)) { /* Read partial block */
            for (n = 64 - 16; n; n--)
              SD_CARD__RECEIVE_BYTE(); /* Purge trailing data */
            *(DWORD *)buff = 16UL << (csd[10] >> 4);
            res = RES_OK;
          }
        }
      } else {                                                                           /* SDC ver 1.XX or MMC */
        if ((sd_card__send_command(CMD9, 0) == 0) && sd_card__read_datablock(csd, 16)) { /* Read CSD */
          if (g_card_type & CT_SD1) {                                                    /* SDC ver 1.XX */
            *(DWORD *)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
          } else { /* MMC */
            *(DWORD *)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
          }
          res = RES_OK;
        }
      }
      break;

    case MMC_GET_TYPE: /* Get card type flags (1 byte) */
      *ptr = g_card_type;
      res = RES_OK;
      break;

    case MMC_GET_CSD:                         /* Receive CSD as a data block (16 bytes) */
      if (sd_card__send_command(CMD9, 0) == 0 /* READ_CSD */
          && sd_card__read_datablock(ptr, 16))
        res = RES_OK;
      break;

    case MMC_GET_CID:                          /* Receive CID as a data block (16 bytes) */
      if (sd_card__send_command(CMD10, 0) == 0 /* READ_CID */
          && sd_card__read_datablock(ptr, 16))
        res = RES_OK;
      break;

    case MMC_GET_OCR:                             /* Receive OCR as an R3 resp (4 bytes) */
      if (sd_card__send_command(CMD58, 0) == 0) { /* READ_OCR */
        for (n = 4; n; n--)
          *ptr++ = SD_CARD__RECEIVE_BYTE();
        res = RES_OK;
      }
      break;

    case MMC_GET_SDSTAT:                           /* Receive SD status as a data block (64 bytes) */
      if (sd_card__send_command(ACMD13, 0) == 0) { /* SD_STATUS */
        SD_CARD__RECEIVE_BYTE();
        if (sd_card__read_datablock(ptr, 64))
          res = RES_OK;
      }
      break;

    default:
      res = RES_PARERR;
      break;
    }

    SD_CARD__DESELECT();
  }

  return res;
}
#endif /* SD_CARD__USE_IOCTL != 0 */

void sd_card__update_status(void) {
  BYTE s = g_disk_status;

  if (SD_CARD__IS_PRESENT()) {
    s &= ~STA_NODISK;
  } else {
    s |= (STA_NODISK | STA_NOINIT);
  }

  g_disk_status = s;
}
