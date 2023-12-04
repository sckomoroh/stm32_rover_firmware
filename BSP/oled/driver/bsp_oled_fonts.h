/*
 * bsp_oled_fonts.h
 *
 *  Created on: Dec 1, 2023
 *      Author: yzvihunov
 */

#ifndef OLED_BSP_OLED_FONTS_H_
#define OLED_BSP_OLED_FONTS_H_


#include <stdint.h>
#include <string.h>

typedef struct {
	uint8_t font_width;
	uint8_t font_height;
	const uint16_t *data;
} font_def_t;


extern font_def_t font_7x10;

extern font_def_t font_11x18;

extern font_def_t font_16x26;

#endif /* OLED_BSP_OLED_FONTS_H_ */
