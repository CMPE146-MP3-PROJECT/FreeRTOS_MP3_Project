/**
 * @file
 * FreeRTOS semphr.h stub replacement for unit-testing
 *
 * This is needed because CMock cannot mock the real FreeRTOS semphr.h
 * because of the way FreeRTOS performs #includes (yuck)
 *
 * This file is picked up by the UT framework BEFORE it searches the
 * FreeRTOS source code folder for semphr.h
 */
#pragma once

#include "portmacro.h"

/*******************************************************************************
 *
 *                               D E F I N E S
 *
 ******************************************************************************/

/*******************************************************************************
 *
 *                                 E N U M S
 *
 ******************************************************************************/

/*******************************************************************************
 *
 *                               T Y P E D E F S
 *
 ******************************************************************************/

typedef void *SemaphoreHandle_t;
typedef void *StaticSemaphore_t;

/*******************************************************************************
 *
 *                      P U B L I C    F U N C T I O N S
 *
 ******************************************************************************/

SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t *pxSemaphoreBuffer);
SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *pxMutexBuffer);

BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait);
BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore);
