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
__base_flash     = 0x00000000;
__top_flash      = __base_flash + 512k;

__base_ram_64k  = 0x10000000;
__top_ram_64k   = __base_ram_64k + 64k;

__base_ram_32k = 0x20000000;
__top_ram_32k  = __base_ram_32k + 32k;

ENTRY(entry_point)

SECTIONS
{
    /* MAIN TEXT SECTION */
    .text : ALIGN(4)
    {
        FILL(0xff)
        __vectors_start__ = ABSOLUTE(.) ;
        KEEP(*(.interrupt_vector_table))

        /* Global Section Table */
        . = ALIGN(4) ;
        section_table_start = .;
        data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        LONG(LOADADDR(.data_ram2));
        LONG(    ADDR(.data_ram2));
        LONG(  SIZEOF(.data_ram2));
        data_section_table_end = .;
        bss_section_table = .;

        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        LONG(    ADDR(.bss_ram2));
        LONG(  SIZEOF(.bss_ram2));
        bss_section_table_end = .;
        section_table_end = . ;
        /* End of Global Section Table */

        *(.after_vectors*)

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

    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*)
         . = ALIGN(4) ;
        _end_noinit = .;
    } > ram_64k

    /* DATA section */
    .data : ALIGN(4)
    {
       FILL(0xff)
       *(vtable)
       *(.ramfunc*)
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

    /* NOINIT and PERSISTENT RAM section for ram_32k */
    .noinit_ram2 (NOLOAD) : ALIGN(4)
    {
        KEEP(*(.persistent_ram))
        persistent_ram = .;

        KEEP(*(._noinit_ram2))
        _noinit_ram2 = .;
        *(.noinit_ram2*)
       . = ALIGN(4) ;
    } > ram_32k

    /* DATA section for ram_32k */
    .data_ram2 : ALIGN(4)
    {
        FILL(0xff)
        *(.ramfunc.$ram2)
        *(.ramfunc.$ram_32k)
        *(.data.$ram2*)
        *(.data.$ram_32k*)
        . = ALIGN(4) ;
        PROVIDE(_data_end_ram2 = .) ;
     } > ram_32k AT>flash_512k

    /* BSS section for ram_32k */
    .bss_ram2 : ALIGN(4)
    {
        _bss_start_ram2 = .;
       *(.bss.$ram2*)
       *(.bss.$ram_32k*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(_bss_end_ram2 = .) ;
    } > ram_32k

    /* Heap starts at the beginning of ram2 after any entities that occupy the beginning of ram2 such as persistent_ram */
    PROVIDE(_heap_start = .);
    PROVIDE(_heap_end = __top_ram_32k);

    /* Stack is at the top of RAM64 unless explicitly defined by the user */
    PROVIDE(_estack = DEFINED(__user_stack_top) ? __user_stack_top : __top_ram_64k - 0);
}

PROVIDE(_bdata_lma = LOADADDR(.data));
PROVIDE(_bdata_vma = ADDR(.data));

PROVIDE(_bdata_lma_peripheral = LOADADDR(.data_ram2));
PROVIDE(_bdata_vma_peripheral = ADDR(.data_ram2));