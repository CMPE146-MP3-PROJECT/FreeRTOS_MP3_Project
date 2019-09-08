/**
 * RAM initialization
 *
 * This module is responsible for RAM initialization implementation and is used by the startup module.
 * Do not invoke in user application.
 */

#pragma once

void ram__init_data(void);
void ram__init_bss(void);
