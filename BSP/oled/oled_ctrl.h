/*
 *  Created on: Dec 4, 2023
 *      Author: Yehor Zvihunov
 */

#pragma once

#include <stdint.h>

void bsp_oled_init();

void bsp_oled_process();

void bsp_oled_set_string(uint8_t line_num, char* text);

void bsp_oled_clear();

void bsp_oled_process_cmd(void* data);
