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
  extern void *_edata;

  uint8_t *src = (uint8_t *)&_bdata_lma;
  uint8_t *dest = (uint8_t *)&_bdata_vma;
  while (dest < (uint8_t *)&_edata) {
    *dest = *src;
    dest++;
    src++;
  }
}

static void ram__init_data_sram_peripheral(void) {
  extern void *_bdata_lma_peripheral;
  extern void *_bdata_vma_peripheral;
  extern void *_edata_peripheral;

  uint8_t *src = (uint8_t *)&_bdata_lma_peripheral;
  uint8_t *dest = (uint8_t *)&_bdata_vma_peripheral;
  while (dest < (uint8_t *)&_edata_peripheral) {
    *dest = *src;
    dest++;
    src++;
  }
}

static void ram__init_bss_sram_main(void) {
  extern void *_bbss;
  extern void *_ebss;

  uint8_t *ptr = (uint8_t *)&_bbss;
  while (ptr < (uint8_t *)&_ebss) {
    *ptr = 0U;
    ptr++;
  }
}

static void ram__init_bss_sram_peripheral(void) {
  extern void *_bbss_peripheral;
  extern void *_ebss_peripheral;

  uint8_t *ptr = (uint8_t *)&_bbss_peripheral;
  while (ptr < (uint8_t *)&_ebss_peripheral) {
    *ptr = 0U;
    ptr++;
  }
}
