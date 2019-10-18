#include <stdint.h>

#include "ram.h"

static void ram__init_data_sram_main(void);
static void ram__init_data_sram_peripheral(void);
static void ram__init_bss_sram_main(void);
static void ram__init_bss_sram_peripheral(void);

void ram__init_data(void) {
  ram__init_data_sram_main();
  ram__init_data_sram_peripheral();
}

void ram__init_bss(void) {
  ram__init_bss_sram_main();
  ram__init_bss_sram_peripheral();
}

static void ram__init_data_sram_main(void) {
  extern void *_bdata_lma;
  extern void *_bdata_vma;
  extern void *_data_end;

  uint8_t *src = (uint8_t *)&_bdata_lma;
  uint8_t *dest = (uint8_t *)&_bdata_vma;
  while (dest < (uint8_t *)&_data_end) {
    *dest = *src;
    dest++;
    src++;
  }
}

static void ram__init_data_sram_peripheral(void) {
  extern void *_bdata_lma_peripheral;
  extern void *_bdata_vma_peripheral;
  extern void *_data_end_ram2;

  uint8_t *src = (uint8_t *)&_bdata_lma_peripheral;
  uint8_t *dest = (uint8_t *)&_bdata_vma_peripheral;
  while (dest < (uint8_t *)&_data_end_ram2) {
    *dest = *src;
    dest++;
    src++;
  }
}

static void ram__init_bss_sram_main(void) {
  extern void *_bss_start;
  extern void *_bss_end;

  uint8_t *ptr = (uint8_t *)&_bss_start;
  while (ptr < (uint8_t *)&_bss_end) {
    *ptr = 0U;
    ptr++;
  }
}

static void ram__init_bss_sram_peripheral(void) {
  extern void *_bss_start_ram2;
  extern void *_bss_end_ram2;

  uint8_t *ptr = (uint8_t *)&_bss_start_ram2;
  while (ptr < (uint8_t *)&_bss_end_ram2) {
    *ptr = 0U;
    ptr++;
  }
}
