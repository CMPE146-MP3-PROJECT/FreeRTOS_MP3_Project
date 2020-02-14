/**
 * @file
 * FreeRTOS task.h stub replacement for unit-testing
 *
 * This is needed because CMock cannot mock the real FreeRTOS task.h
 * because of the way FreeRTOS performs #includes (yuck)
 *
 * This file is picked up by the UT framework BEFORE it searches the
 * FreeRTOS source code folder for task.h
 */
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "portmacro.h"

/*******************************************************************************
 *
 *                               D E F I N E S
 *
 ******************************************************************************/
#define taskDISABLE_INTERRUPTS() portDISABLE_INTERRUPTS()
#define taskENABLE_INTERRUPTS() portENABLE_INTERRUPTS()

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
typedef void *TaskFunction_t;
typedef void *TaskHandle_t;
typedef uint32_t configSTACK_DEPTH_TYPE;

/*******************************************************************************
 *
 *                      P U B L I C    F U N C T I O N S
 *
 ******************************************************************************/

void taskENTER_CRITICAL(void);
void taskEXIT_CRITICAL(void);

TickType_t xTaskGetTickCount(void);
TickType_t xTaskGetTickCountFromISR(void);

void vTaskDelay(const TickType_t xTicksToDelay);
void vTaskDelayUntil(TickType_t *const pxPreviousWakeTime, const TickType_t xTimeIncrement);

BaseType_t xTaskCreate(
    TaskFunction_t pxTaskCode,
    const char *const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
    const configSTACK_DEPTH_TYPE usStackDepth, void *const pvParameters, UBaseType_t uxPriority,
    TaskHandle_t *const pxCreatedTask);

#ifdef __cplusplus
} /* extern "C" */
#endif
