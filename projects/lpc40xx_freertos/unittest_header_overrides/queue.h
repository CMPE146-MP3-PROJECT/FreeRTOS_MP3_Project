/**
 * @file
 * FreeRTOS queue.h stub replacement for unit-testing
 *
 * This is needed because CMock cannot mock the real FreeRTOS queue.h
 * because of the way FreeRTOS performs #includes (yuck)
 *
 * This file is picked up by the UT framework BEFORE it searches the
 * FreeRTOS source code folder for queue.h
 */
#pragma once

#include "portmacro.h"

/*******************************************************************************
 *
 *                               D E F I N E S
 *
 ******************************************************************************/

#define pdTRUE ((BaseType_t)1)
#define pdFALSE ((BaseType_t)0)

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
typedef void *QueueHandle_t;
typedef void *StaticQueue_t;

/*******************************************************************************
 *
 *                      P U B L I C    F U N C T I O N S
 *
 ******************************************************************************/
QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize);
QueueHandle_t xQueueCreateStatic(UBaseType_t uxQueueLength, UBaseType_t uxItemSize, uint8_t *pucQueueStorageBuffer,
                                 StaticQueue_t *pxQueueBuffer);

BaseType_t xQueuePeek(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait);
BaseType_t xQueueReceive(QueueHandle_t xQueue, void *const pvBuffer, TickType_t xTicksToWait);
BaseType_t xQueueSend(QueueHandle_t xQueue, const void *const pvItemToQueue, TickType_t xTicksToWait);
BaseType_t xQueueOverwrite(QueueHandle_t xQueue, const void *pvItemToQueue);
BaseType_t uxQueueMessagesWaitingFromISR(QueueHandle_t xQueue);
BaseType_t uxQueueMessagesWaiting(QueueHandle_t xQueue);
BaseType_t xQueueReceiveFromISR(QueueHandle_t xQueue, void *pvBuffer, BaseType_t *pxHigherPriorityTaskWoken);
BaseType_t xQueueSendFromISR(QueueHandle_t xQueue, const void *pvItemToQueue, BaseType_t *pxHigherPriorityTaskWoken);
BaseType_t xQueueReset(QueueHandle_t xQueue);
