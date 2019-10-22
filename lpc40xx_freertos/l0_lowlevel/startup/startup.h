/**
 * Startup module
 *
 * This module is responsible for implementing platform startup and is used at startup before main().
 */

#pragma once

void startup__initialize_ram(void);
void startup__initialize_fpu(void);
void startup__initialize_interrupts(void);
