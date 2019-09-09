#pragma once

#include <stdint.h>

/**
 * @{
 * @name delay time API
 *
 * The delay API is designed to be precise, with or without the OS
 * If the OS is running, milliseconds will be slept, and microseconds will be polled (by eating away the CPU)
 */
void delay_ms(uint32_t ms);
void delay_us(uint64_t us);
/** @} */
