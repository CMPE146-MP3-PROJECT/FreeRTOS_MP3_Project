#include "lpc40xx.h"

int main(void)
{
    LPC_GPIO1->DIR |= (1 << 26);

    while (1 == 1) {
        LPC_GPIO1->PIN ^= (1 << 26);
        for (volatile unsigned int i = 0; i < 100000; i++);
    }

    return -1;
}
