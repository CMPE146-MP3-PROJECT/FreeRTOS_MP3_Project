#include "lpc40xx.h"

#include "FreeRTOS.h"
#include "task.h"

static void task(void *params);
static void task1(void *params);

int main(void)
{
    xTaskCreate((TaskFunction_t)task, "task", 512U, NULL, PRIORITY_HIGH, NULL);
    xTaskCreate((TaskFunction_t)task1, "task1", 512U, NULL, PRIORITY_LOW, NULL);
    vTaskStartScheduler();

    /**
     * vTaskStartScheduler() should never return.
     * Otherwise, it returning indicates there is not enough free memory in RAM or scheduler was explicitly terminated.
     * CPU will now halt forever at this point.
     */
    while (1 == 1) {}

    return -1;
}

static void task(void *params)
{
    LPC_GPIO1->DIR |= (1 << 26);

    while (1 == 1) {
        LPC_GPIO1->PIN ^= (1 << 26);
        vTaskDelay(1000U);
    }
}

static void task1(void *params)
{
    LPC_GPIO2->DIR |= (1 << 3);

    while (1 == 1) {
        LPC_GPIO2->PIN ^= (1 << 3);
        vTaskDelay(1000U);
    }
}
