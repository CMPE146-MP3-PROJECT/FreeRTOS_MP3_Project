#pragma once

#include <stdint.h>

/**
 * @{
 * @name delay time API
 * @warning This API needs sys_time.h to be initialized
 *
 * The delay API is designed to be precise, with or without the OS
 * If the OS is running, milliseconds will be slept, and microseconds will be polled (by eating away the CPU)
 */
void delay__ms(uint32_t ms);
void delay__us(uint64_t us);
/** @} */
