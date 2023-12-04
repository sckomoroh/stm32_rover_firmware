/*
 * bsp_ssd1306.h
 *
 *  Created on: Dec 1, 2023
 *      Author: yzvihunov
 */

#ifndef OLED_BSP_SSD1306_H_
#define OLED_BSP_SSD1306_H_

#include "bsp_oled_fonts.h"
#include <stdint.h>


#define SSD1306_I2C_ADDR         0x3C
#define SSD1306_WIDTH            128
#define SSD1306_HEIGHT           32

typedef enum {
	SSD1306_COLOR_BLACK = 0x00, /*!< Black color, no pixel */
	SSD1306_COLOR_WHITE = 0x01 /*!< Pixel is set. Color depends on LCD */
} SSD1306_COLOR_t;

uint8_t ssd1306_init(void);
void ssd1306_update_screen(void);
void ssd1306_toggle_invert(void);
void ssd1306_fill(SSD1306_COLOR_t color);
void ssd1306_draw_pixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color);
void ssd1306_goto_xy(uint16_t x, uint16_t y);
char ssd1306_putc(char ch, font_def_t *font, SSD1306_COLOR_t color);
char ssd1306_puts(char *str, font_def_t *font, SSD1306_COLOR_t color);
void ssd1306_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
		SSD1306_COLOR_t c);
void ssd1306_draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
		SSD1306_COLOR_t c);
void ssd1306_draw_filled_rectangle(uint16_t x, uint16_t y, uint16_t w,
		uint16_t h, SSD1306_COLOR_t c);
void ssd1306_draw_triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
		uint16_t x3, uint16_t y3, SSD1306_COLOR_t color);
void ssd1306_draw_circle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c);
void ssd1306_draw_filled_circle(int16_t x0, int16_t y0, int16_t r,
		SSD1306_COLOR_t c);

void ssd1306_i2c_write(uint8_t address, uint8_t reg, uint8_t data);
void ssd1306_i2c_write_arr(uint8_t address, uint8_t reg, uint8_t *data,
		uint16_t count);

#endif /* OLED_BSP_SSD1306_H_ */
