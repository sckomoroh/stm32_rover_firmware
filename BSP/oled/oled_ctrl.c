/*
 *  Created on: Dec 4, 2023
 *      Author: Yehor Zvihunov
 */

#include "oled_ctrl.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "driver/bsp_ssd1306.h"
#include "common.h"

#include "uart/uart_ctrl.h"

#define OLED_STRING_SIZE 24

struct
{
    boot_t update;
    char *lines[3];
} _oled_data;

struct
{
    uint8_t line_number;
    char text[OLED_STRING_SIZE];
} _oled_params;

static void _oled_clear();
static void _oled_refresh();
static void _oled_draw_string(char *data, uint8_t x, uint8_t y);

void bsp_oled_init()
{
    _oled_data.update = FALSE;
    for (int i = 0; i < 3; i++)
    {
        _oled_data.lines[i] = (char *)malloc(OLED_STRING_SIZE);
        memset(_oled_data.lines[i], 0, OLED_STRING_SIZE);
    }

    ssd1306_init();
}

void bsp_oled_process()
{
    static uint8_t i;

    if (_oled_data.update == TRUE)
    {
        _oled_data.update = FALSE;
        _oled_clear();

        for (i = 0; i < 3; i++)
        {
            _oled_draw_string(_oled_data.lines[i], 0, 10 * i);
        }

        _oled_refresh();
    }
}

void bsp_oled_set_string(uint8_t line_num, char *text)
{
    if (line_num < 3)
    {
        _oled_data.update = TRUE;
        memset(_oled_data.lines[line_num], 0, OLED_STRING_SIZE);
        strcpy(_oled_data.lines[line_num], text);
    }
}

void bsp_oled_clear()
{
    _oled_data.update = TRUE;
    for (int i = 0; i < 3; i++)
    {
        memset(_oled_data.lines[i], 0, OLED_STRING_SIZE);
    }
}

void bsp_oled_process_cmd(void *data)
{
    memset(&_oled_params, 0, sizeof(_oled_params));
    memcpy(&_oled_params, data, sizeof(_oled_params));

    bsp_oled_set_string(_oled_params.line_number, _oled_params.text);
}

static void _oled_clear()
{
    ssd1306_fill(SSD1306_COLOR_BLACK);
}

static void _oled_refresh()
{
    ssd1306_update_screen();
}

static void _oled_draw_string(char *data, uint8_t x, uint8_t y)
{
    ssd1306_goto_xy(x, y);
    ssd1306_puts(data, &font_7x10, SSD1306_COLOR_WHITE);
}
