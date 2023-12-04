#ifndef OLED_BSP_SSD1306_H_
#define OLED_BSP_SSD1306_H_

#include <stdint.h>

#include "bsp_oled_fonts.h"

#define SSD1306_I2C_ADDR 0x3C
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 32

typedef enum {
    SSD1306_COLOR_BLACK = 0x00, /*!< Black color, no pixel */
    SSD1306_COLOR_WHITE = 0x01  /*!< Pixel is set. Color depends on LCD */
} SSD1306_COLOR_t;

uint8_t ssd1306_init(void);
void ssd1306_update_screen(void);
void ssd1306_fill(SSD1306_COLOR_t color);
void ssd1306_goto_xy(uint16_t x, uint16_t y);
char ssd1306_puts(char* str, font_def_t* font, SSD1306_COLOR_t color);

#endif /* OLED_BSP_SSD1306_H_ */
