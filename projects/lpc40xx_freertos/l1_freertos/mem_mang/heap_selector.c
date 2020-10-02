#include "FreeRTOSConfig.h"

// See:
// https://www.freertos.org/a00111.html

// Default the choice
#ifndef configMEMMANG_SELECTION
#define configMEMMANG_SELECTION 3
#endif

/**
 * All the "heap" implementation files below were re-named from FreeRTOS source *.c to *.inc
 * *.inc is not included in the scons build system so this is a trick
 * This *.c file is built, and #includes the implementation as per configMEMMANG_SELECTION
 */
#if (1 == configMEMMANG_SELECTION)
#include "heap_1.c.inc"
#elif (2 == configMEMMANG_SELECTION)
#include "heap_2.c.inc"
#elif (3 == configMEMMANG_SELECTION)
#include "heap_3.c.inc"
#elif (4 == configMEMMANG_SELECTION)
#include "heap_4.c.inc"
#else
#include "heap_5.c.inc"
#endif
