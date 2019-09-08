#include <stdint.h>

#include "ram.h"

static void ram__init_data_sram_main(void);
static void ram__init_data_sram_peripheral(void);
static void ram__init_bss_sram_main(void);
static void ram__init_bss_sram_peripheral(void);

void ram__init_data(void)
{
    ram__init_data_sram_main();
    ram__init_data_sram_peripheral();
}

void ram__init_bss(void)
{
    ram__init_bss_sram_main();
    ram__init_bss_sram_peripheral();
}

static void ram__init_data_sram_main(void)
{
    extern uint8_t *_bdata_lma;
    extern uint8_t *_bdata_vma;
    extern uint8_t *_edata;

    const uint8_t *src = _bdata_lma;
    uint8_t *dest = _bdata_vma;
    while (src < _edata) {
        *dest = *src;
        dest++;
        src++;
    }
}

static void ram__init_data_sram_peripheral(void)
{
    extern uint8_t *_bdata_lma_peripheral;
    extern uint8_t *_bdata_vma_peripheral;
    extern uint8_t *_edata_peripheral;

    const uint8_t *src = _bdata_lma_peripheral;
    uint8_t *dest = _bdata_vma_peripheral;
    while (src < _edata_peripheral) {
        *dest = *src;
        dest++;
        src++;
    }
}

static void ram__init_bss_sram_main(void)
{
    extern uint8_t *_bbss;
    extern uint8_t *_ebss;

    uint8_t *ptr = _bbss;
    while (ptr < _ebss) {
        *ptr = 0U;
        ptr++;
    }
}

static void ram__init_bss_sram_peripheral(void)
{
    extern uint8_t *_bbss_peripheral;
    extern uint8_t *_ebss_peripheral;

    uint8_t *ptr = _bbss_peripheral;
    while (ptr < _ebss_peripheral) {
        *ptr = 0U;
        ptr++;
    }
}
