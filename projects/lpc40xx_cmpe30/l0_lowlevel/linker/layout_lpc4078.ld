/**
 * References:
 * - https://sourceware.org/binutils/docs/ld/Basic-Script-Concepts.html#Basic-Script-Concepts
 * - https://arobenko.gitbooks.io/bare_metal_cpp/content/
 */

MEMORY
{
  flash_512k (rx) : ORIGIN = 0x00000000, LENGTH = 512K
  ram_64k (rwx)   : ORIGIN = 0x10000000, LENGTH = 64K   /* Main RAM       */
  ram_32k (rwx)   : ORIGIN = 0x20000000, LENGTH = 32K   /* Peripheral RAM */
}

/*************************************************
                                                  *
     64K RAM +-----Stack Top   +-----+ 32K RAM    *
             |    ||           |     |            *
             |    \/           |     |            *
             |     |           |     |            *
             |     |           |     |            *
             |    /\ heap      |    /\            *
             |    || overflow  |    ||            *
             .-----.           +-----Heap Start   *
             |bbbbb|           0x2000000          *
             |bbbbb| *bss                         *
             .-----.                              *
             |ddddd|                              *
             |ddddd| *data                        *
             .-----.                              *
            0x1000000                             *
                                                  *
**************************************************
*/

/* Define a symbol for the top of each memory region */
__base_flash     = ORIGIN(flash_512k);
__top_flash      = __base_flash + LENGTH(flash_512k);

__base_ram_64k  = ORIGIN(ram_64k);
__top_ram_64k   = __base_ram_64k + LENGTH(ram_64k);

__base_ram_32k = ORIGIN(ram_32k);
__top_ram_32k  = __base_ram_32k + LENGTH(ram_32k);

ENTRY(entry_point)

SECTIONS
{
    .text : ALIGN(4)
    {
        FILL(0xff)
        KEEP(*(.interrupt_vector_table))

        /* C++ constructors etc */
        . = ALIGN(4);
        KEEP(*(.init))

        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP (*(.preinit_array))
        __preinit_array_end = .;

        . = ALIGN(4);
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;

        KEEP(*(.fini));

        . = ALIGN(4);
        KEEP (*crtbegin.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*crtend.o(.ctors))

        . = ALIGN(4);
        KEEP (*crtbegin.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*crtend.o(.dtors))
        /* End C++ */
    } >flash_512k

    .text : ALIGN(4)
    {
        *(.text*)
        *(.rodata .rodata.* .constdata .constdata.*)
        . = ALIGN(4);
    } > flash_512k

    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this.
     */
    .ARM.extab : ALIGN(4)
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > flash_512k
    __exidx_start = .;

    .ARM.exidx : ALIGN(4)
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > flash_512k
    __exidx_end = .;

    _etext = .;

    /* DATA section */
    .data : ALIGN(4)
    {
       FILL(0xff)
       *(vtable)
       *(.data*)
       . = ALIGN(4) ;
       PROVIDE(_data_end = .);
    } > ram_64k AT>flash_512k

    /* BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        PROVIDE(_bss_end = .);
        PROVIDE(end = .);
    } > ram_64k

    /* At this point, we have un-used RAM after the BSS and DATA section all the way to the top */
    PROVIDE(__start_of_unused_ram64 = .);
    PROVIDE(__end_of_unused_ram64 = __top_ram_64k);

    /* PERSISTENT RAM section for ram_32k */
    .persistent_ram2 (NOLOAD) : ALIGN(4)
    {
        *(.persistent_ram)
       . = ALIGN(4) ;
    } > ram_32k

    /* Heap starts at the beginning of ram2 after any entities that occupy the beginning of ram2 such as persistent_ram */
    PROVIDE(_heap_start = .);
    PROVIDE(_heap_end = __top_ram_32k);
}

/* Stack is at the top of RAM64 unless explicitly defined by the user */
PROVIDE(_estack = DEFINED(__user_stack_top) ? __user_stack_top : __top_ram_64k);

PROVIDE(_bdata_lma = LOADADDR(.data));
PROVIDE(_bdata_vma = ADDR(.data));
