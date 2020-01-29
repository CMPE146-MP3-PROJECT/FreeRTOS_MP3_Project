/**
 * @file
 * FreeRTOS.h stub replacement for unit-testing
 *
 * This is needed because CMock cannot mock the real FreeRTOS
 * because of the way FreeRTOS performs #includes (yuck)
 *
 * This file is picked up by the UT framework BEFORE it searches the
 * FreeRTOS source code folder for semphr.h
 */

#ifndef INC_FREERTOS_H
#define INC_FREERTOS_H

/* Application specific configuration options. */
#include "FreeRTOSConfig.h"
#endif /* INC_FREERTOS_H */
