#include "ram.h"

void startup__initialize_ram(void)
{
    ram__init_data();
    ram__init_bss();
}

void startup__initialize_system_clock(void)
{
    // TODO: Change default core clock
}
