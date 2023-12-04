/*
 * bsp_ssd1306.c
 *
 *  Created on: Dec 1, 2023
 *      Author: yzvihunov
 */

#include "bsp_io_i2c.h"
#include "bsp_ssd1306.h"

#define SSD1306_WRITECOMMAND(command) ssd1306_i2c_write(SSD1306_I2C_ADDR, 0x00, (command))
#define SSD1306_WRITEDATA(data) ssd1306_i2c_write(SSD1306_I2C_ADDR, 0x40, (data))

static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

typedef struct {
	uint16_t current_x;
	uint16_t current_y;
	uint8_t inverted;
	uint8_t initialized;
} SSD1306_t;

static SSD1306_t SSD1306;

static uint16_t ABS(int value) {
	int temp = value;
	if (value < 0) {
		temp = -value;
	}

	return temp;
}

uint8_t ssd1306_init(void) {
	uint32_t p = 2500;
	while (p > 0) {
		p--;
	}

	SSD1306_WRITECOMMAND(0xae);		      // display off
	SSD1306_WRITECOMMAND(0xa6);          // Set Normal Display (default)
	SSD1306_WRITECOMMAND(0xAE);        	// DISPLAYOFF
	SSD1306_WRITECOMMAND(0xD5);        	// SETDISPLAYCLOCKDIV
	SSD1306_WRITECOMMAND(0x80);        	// the suggested ratio 0x80
	SSD1306_WRITECOMMAND(0xA8);        	// SSD1306_SETMULTIPLEX
	SSD1306_WRITECOMMAND(0x1F);
	SSD1306_WRITECOMMAND(0xD3);        	// SETDISPLAYOFFSET
	SSD1306_WRITECOMMAND(0x00);         	// no offset
	SSD1306_WRITECOMMAND(0x40 | 0x0);  	// SETSTARTLINE
	SSD1306_WRITECOMMAND(0x8D);        	// CHARGEPUMP
	SSD1306_WRITECOMMAND(0x14);          // 0x014 enable, 0x010 disable
	SSD1306_WRITECOMMAND(0x20); // com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5),
	SSD1306_WRITECOMMAND(0x02); // 0x12 //128x32 OLED: 0x002,  128x32 OLED 0x012
	SSD1306_WRITECOMMAND(0xa1);          // segment remap a0/a1
	SSD1306_WRITECOMMAND(0xc8);          // c0: scan dir normal, c8: reverse
	SSD1306_WRITECOMMAND(0xda);
	SSD1306_WRITECOMMAND(0x02); // com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5)
	SSD1306_WRITECOMMAND(0x81);
	SSD1306_WRITECOMMAND(0xcf);          // [2] set contrast control
	SSD1306_WRITECOMMAND(0xd9);
	SSD1306_WRITECOMMAND(0xf1);          // [2] pre-charge period 0x022/f1
	SSD1306_WRITECOMMAND(0xdb);
	SSD1306_WRITECOMMAND(0x40);          // vcomh deselect level
	SSD1306_WRITECOMMAND(0x2e);          // Disable scroll
	SSD1306_WRITECOMMAND(0xa4);          // output ram to display
	SSD1306_WRITECOMMAND(0xa6);          // none inverted normal display mode
	SSD1306_WRITECOMMAND(0xaf);          // display on

	ssd1306_fill(SSD1306_COLOR_BLACK);

	ssd1306_update_screen();

	SSD1306.current_x = 0;
	SSD1306.current_y = 0;

	SSD1306.initialized = 1;

	return 1;
}

void ssd1306_update_screen(void) {
	uint8_t m;

	for (m = 0; m < 8; m++) {
		SSD1306_WRITECOMMAND(0xB0 + m);
		SSD1306_WRITECOMMAND(0x00);
		SSD1306_WRITECOMMAND(0x10);

		ssd1306_i2c_write_arr(SSD1306_I2C_ADDR, 0x40,
				&SSD1306_Buffer[SSD1306_WIDTH * m], SSD1306_WIDTH);
	}
}

void ssd1306_toggle_invert(void) {
	uint16_t i;

	SSD1306.inverted = !SSD1306.inverted;

	for (i = 0; i < sizeof(SSD1306_Buffer); i++) {
		SSD1306_Buffer[i] = ~SSD1306_Buffer[i];
	}
}

void ssd1306_fill(SSD1306_COLOR_t color) {
	memset(SSD1306_Buffer, (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF,
			sizeof(SSD1306_Buffer));
}

void ssd1306_draw_pixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color) {
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
		return;
	}

	if (SSD1306.inverted) {
		color = (SSD1306_COLOR_t) !color;
	}

	if (color == SSD1306_COLOR_WHITE) {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	} else {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}

void ssd1306_goto_xy(uint16_t x, uint16_t y) {
	SSD1306.current_x = x;
	SSD1306.current_y = y;
}

char ssd1306_putc(char ch, font_def_t *font, SSD1306_COLOR_t color) {
	uint32_t i, b, j;

	if (
	SSD1306_WIDTH <= (SSD1306.current_x + font->font_width) ||
	SSD1306_HEIGHT <= (SSD1306.current_y + font->font_height)) {
		return 0;
	}

	for (i = 0; i < font->font_height; i++) {
		b = font->data[(ch - 32) * font->font_height + i];
		for (j = 0; j < font->font_width; j++) {
			if ((b << j) & 0x8000) {
				ssd1306_draw_pixel(SSD1306.current_x + j,
						(SSD1306.current_y + i), (SSD1306_COLOR_t) color);
			} else {
				ssd1306_draw_pixel(SSD1306.current_x + j,
						(SSD1306.current_y + i), (SSD1306_COLOR_t) !color);
			}
		}
	}

	SSD1306.current_x += font->font_width;

	return ch;
}

char ssd1306_puts(char *str, font_def_t *font, SSD1306_COLOR_t color) {
	while (*str) {
		if (ssd1306_putc(*str, font, color) != *str) {
			return *str;
		}

		str++;
	}

	return *str;
}

void ssd1306_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
		SSD1306_COLOR_t color) {
	int16_t dx, dy, sx, sy, err, e2, i, tmp;

	if (x0 >= SSD1306_WIDTH) {
		x0 = SSD1306_WIDTH - 1;
	}

	if (x1 >= SSD1306_WIDTH) {
		x1 = SSD1306_WIDTH - 1;
	}

	if (y0 >= SSD1306_HEIGHT) {
		y0 = SSD1306_HEIGHT - 1;
	}

	if (y1 >= SSD1306_HEIGHT) {
		y1 = SSD1306_HEIGHT - 1;
	}

	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1);
	sx = (x0 < x1) ? 1 : -1;
	sy = (y0 < y1) ? 1 : -1;
	err = ((dx > dy) ? dx : -dy) / 2;

	if (dx == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}

		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}

		/* Vertical line */
		for (i = y0; i <= y1; i++) {
			ssd1306_draw_pixel(x0, i, color);
		}

		return;
	}

	if (dy == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}

		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}

		/* Horizontal line */
		for (i = x0; i <= x1; i++) {
			ssd1306_draw_pixel(i, y0, color);
		}

		return;
	}

	while (1) {
		ssd1306_draw_pixel(x0, y0, color);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

void ssd1306_draw_rectangle(uint16_t x, uint16_t y, uint16_t width,
		uint16_t height, SSD1306_COLOR_t color) {
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
		return;
	}

	if ((x + width) >= SSD1306_WIDTH) {
		width = SSD1306_WIDTH - x;
	}

	if ((y + height) >= SSD1306_HEIGHT) {
		height = SSD1306_HEIGHT - y;
	}

	ssd1306_draw_line(x, y, x + width, y, color); /* Top line */
	ssd1306_draw_line(x, y + height, x + width, y + height, color); /* Bottom line */
	ssd1306_draw_line(x, y, x, y + height, color); /* Left line */
	ssd1306_draw_line(x + width, y, x + width, y + height, color); /* Right line */
}

void ssd1306_draw_filled_rectangle(uint16_t x, uint16_t y, uint16_t width,
		uint16_t height, SSD1306_COLOR_t color) {
	uint8_t i;

	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
		return;
	}

	if ((x + width) >= SSD1306_WIDTH) {
		width = SSD1306_WIDTH - x;
	}

	if ((y + height) >= SSD1306_HEIGHT) {
		height = SSD1306_HEIGHT - y;
	}

	for (i = 0; i <= height; i++) {
		ssd1306_draw_line(x, y + i, x + width, y + i, color);
	}
}

void ssd1306_draw_triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
		uint16_t x3, uint16_t y3, SSD1306_COLOR_t color) {
	ssd1306_draw_line(x1, y1, x2, y2, color);
	ssd1306_draw_line(x2, y2, x3, y3, color);
	ssd1306_draw_line(x3, y3, x1, y1, color);
}

void SSD1306_DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2,
		uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR_t color) {
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, yinc1 =
			0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
			curpixel = 0;

	deltax = ABS(x2 - x1);
	deltay = ABS(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1) {
		xinc1 = 1;
		xinc2 = 1;
	} else {
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) {
		yinc1 = 1;
		yinc2 = 1;
	} else {
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay) {
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	} else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		ssd1306_draw_line(x, y, x3, y3, color);

		num += numadd;
		if (num >= den) {
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}

void ssd1306_draw_circle(int16_t x0, int16_t y0, int16_t radius,
		SSD1306_COLOR_t color) {
	int16_t f = 1 - radius;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * radius;
	int16_t x = 0;
	int16_t y = radius;

	ssd1306_draw_pixel(x0, y0 + radius, color);
	ssd1306_draw_pixel(x0, y0 - radius, color);
	ssd1306_draw_pixel(x0 + radius, y0, color);
	ssd1306_draw_pixel(x0 - radius, y0, color);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x;

		ssd1306_draw_pixel(x0 + x, y0 + y, color);
		ssd1306_draw_pixel(x0 - x, y0 + y, color);
		ssd1306_draw_pixel(x0 + x, y0 - y, color);
		ssd1306_draw_pixel(x0 - x, y0 - y, color);

		ssd1306_draw_pixel(x0 + y, y0 + x, color);
		ssd1306_draw_pixel(x0 - y, y0 + x, color);
		ssd1306_draw_pixel(x0 + y, y0 - x, color);
		ssd1306_draw_pixel(x0 - y, y0 - x, color);
	}
}

void ssd1306_draw_filled_circle(int16_t x0, int16_t y0, int16_t radius,
		SSD1306_COLOR_t color) {
	int16_t f = 1 - radius;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * radius;
	int16_t x = 0;
	int16_t y = radius;

	ssd1306_draw_pixel(x0, y0 + radius, color);
	ssd1306_draw_pixel(x0, y0 - radius, color);
	ssd1306_draw_pixel(x0 + radius, y0, color);
	ssd1306_draw_pixel(x0 - radius, y0, color);
	ssd1306_draw_line(x0 - radius, y0, x0 + radius, y0, color);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		ssd1306_draw_line(x0 - x, y0 + y, x0 + x, y0 + y, color);
		ssd1306_draw_line(x0 + x, y0 - y, x0 - x, y0 - y, color);

		ssd1306_draw_line(x0 + y, y0 + x, x0 - y, y0 + x, color);
		ssd1306_draw_line(x0 + y, y0 - x, x0 - y, y0 - x, color);
	}
}

void SSD1306_ON(void) {
	SSD1306_WRITECOMMAND(0x8D);
	SSD1306_WRITECOMMAND(0x14);
	SSD1306_WRITECOMMAND(0xAF);
}

void SSD1306_OFF(void) {
	SSD1306_WRITECOMMAND(0x8D);
	SSD1306_WRITECOMMAND(0x10);
	SSD1306_WRITECOMMAND(0xAE);
}

void ssd1306_i2c_write_arr(uint8_t address, uint8_t reg, uint8_t *data,
		uint16_t count) {
	iic_write_len(address, reg, count, data);
}

void ssd1306_i2c_write(uint8_t address, uint8_t reg, uint8_t data) {
	iic_write_byte(address, reg, data);
}
