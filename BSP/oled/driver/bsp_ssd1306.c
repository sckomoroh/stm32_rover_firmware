#include "bsp_ssd1306.h"
#include "bsp_io_i2c.h"

void ssd1306_i2c_write(uint8_t address, uint8_t reg, uint8_t data);
void ssd1306_i2c_write_arr(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count);

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

static void _ssd1306_draw_pixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color);
static char _ssd1306_putc(char ch, font_def_t* font, SSD1306_COLOR_t color);

uint8_t ssd1306_init(void)
{
    uint32_t p = 2500;
    while (p > 0) {
        p--;
    }

    SSD1306_WRITECOMMAND(0xae);  // display off
    SSD1306_WRITECOMMAND(0xa6);  // Set Normal Display (default)
    SSD1306_WRITECOMMAND(0xAE);  // DISPLAYOFF
    SSD1306_WRITECOMMAND(0xD5);  // SETDISPLAYCLOCKDIV
    SSD1306_WRITECOMMAND(0x80);  // the suggested ratio 0x80
    SSD1306_WRITECOMMAND(0xA8);  // SSD1306_SETMULTIPLEX
    SSD1306_WRITECOMMAND(0x1F);
    SSD1306_WRITECOMMAND(0xD3);        // SETDISPLAYOFFSET
    SSD1306_WRITECOMMAND(0x00);        // no offset
    SSD1306_WRITECOMMAND(0x40 | 0x0);  // SETSTARTLINE
    SSD1306_WRITECOMMAND(0x8D);        // CHARGEPUMP
    SSD1306_WRITECOMMAND(0x14);        // 0x014 enable, 0x010 disable
    SSD1306_WRITECOMMAND(0x20);  // com pin HW config, sequential com pin config (bit 4), disable
                                 // left/right remap (bit 5),
    SSD1306_WRITECOMMAND(0x02);  // 0x12 //128x32 OLED: 0x002,  128x32 OLED 0x012
    SSD1306_WRITECOMMAND(0xa1);  // segment remap a0/a1
    SSD1306_WRITECOMMAND(0xc8);  // c0: scan dir normal, c8: reverse
    SSD1306_WRITECOMMAND(0xda);
    SSD1306_WRITECOMMAND(0x02);  // com pin HW config, sequential com pin config (bit 4), disable
                                 // left/right remap (bit 5)
    SSD1306_WRITECOMMAND(0x81);
    SSD1306_WRITECOMMAND(0xcf);  // [2] set contrast control
    SSD1306_WRITECOMMAND(0xd9);
    SSD1306_WRITECOMMAND(0xf1);  // [2] pre-charge period 0x022/f1
    SSD1306_WRITECOMMAND(0xdb);
    SSD1306_WRITECOMMAND(0x40);  // vcomh deselect level
    SSD1306_WRITECOMMAND(0x2e);  // Disable scroll
    SSD1306_WRITECOMMAND(0xa4);  // output ram to display
    SSD1306_WRITECOMMAND(0xa6);  // none inverted normal display mode
    SSD1306_WRITECOMMAND(0xaf);  // display on

    ssd1306_fill(SSD1306_COLOR_BLACK);

    ssd1306_update_screen();

    SSD1306.current_x = 0;
    SSD1306.current_y = 0;

    SSD1306.initialized = 1;

    return 1;
}

void ssd1306_update_screen(void)
{
    uint8_t m;

    for (m = 0; m < 8; m++) {
        SSD1306_WRITECOMMAND(0xB0 + m);
        SSD1306_WRITECOMMAND(0x00);
        SSD1306_WRITECOMMAND(0x10);

        ssd1306_i2c_write_arr(SSD1306_I2C_ADDR, 0x40, &SSD1306_Buffer[SSD1306_WIDTH * m],
                              SSD1306_WIDTH);
    }
}

void ssd1306_fill(SSD1306_COLOR_t color)
{
    memset(SSD1306_Buffer, (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SSD1306_Buffer));
}

void ssd1306_goto_xy(uint16_t x, uint16_t y)
{
    SSD1306.current_x = x;
    SSD1306.current_y = y;
}

char ssd1306_puts(char* str, font_def_t* font, SSD1306_COLOR_t color)
{
    while (*str) {
        if (_ssd1306_putc(*str, font, color) != *str) {
            return *str;
        }

        str++;
    }

    return *str;
}

void SSD1306_ON(void)
{
    SSD1306_WRITECOMMAND(0x8D);
    SSD1306_WRITECOMMAND(0x14);
    SSD1306_WRITECOMMAND(0xAF);
}

void SSD1306_OFF(void)
{
    SSD1306_WRITECOMMAND(0x8D);
    SSD1306_WRITECOMMAND(0x10);
    SSD1306_WRITECOMMAND(0xAE);
}

void ssd1306_i2c_write_arr(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count)
{
    iic_write_len(address, reg, count, data);
}

void ssd1306_i2c_write(uint8_t address, uint8_t reg, uint8_t data)
{
    iic_write_byte(address, reg, data);
}

static void _ssd1306_draw_pixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return;
    }

    if (SSD1306.inverted) {
        color = (SSD1306_COLOR_t)!color;
    }

    if (color == SSD1306_COLOR_WHITE) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    }
    else {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

static char _ssd1306_putc(char ch, font_def_t* font, SSD1306_COLOR_t color)
{
    uint32_t i, b, j;

    if (SSD1306_WIDTH <= (SSD1306.current_x + font->font_width) ||
        SSD1306_HEIGHT <= (SSD1306.current_y + font->font_height)) {
        return 0;
    }

    for (i = 0; i < font->font_height; i++) {
        b = font->data[(ch - 32) * font->font_height + i];
        for (j = 0; j < font->font_width; j++) {
            if ((b << j) & 0x8000) {
                _ssd1306_draw_pixel(SSD1306.current_x + j, (SSD1306.current_y + i),
                                    (SSD1306_COLOR_t)color);
            }
            else {
                _ssd1306_draw_pixel(SSD1306.current_x + j, (SSD1306.current_y + i),
                                    (SSD1306_COLOR_t)!color);
            }
        }
    }

    SSD1306.current_x += font->font_width;

    return ch;
}
