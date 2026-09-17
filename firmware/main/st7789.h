/*
 * DT022CTFT (ST7789H2) driver for the X-NUCLEO-GFX01M2 shield,
 * 240x320 pixels, RGB565. Pure SPI, no dependencies.
 *
 * Wiring on the NUCLEO-L476RG (see assets/pinout-nucleo-gfx01m2.svg):
 *   SPI1:  SCK=PA5   MISO=PA6   MOSI=PA7
 *   CS=PA9 (chip select, active low)
 *   DC=PB10 (data/command, 0=command 1=data)
 *   RST=PA1 (reset, active low)
 */
#pragma once
#include <stdint.h>
#include "regs.h"

#define LCD_WIDTH  240
#define LCD_HEIGHT 320

void lcd_init(void);
void lcd_fill(uint16_t color);
void lcd_fill_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void lcd_pixel(uint16_t x, uint16_t y, uint16_t color);
void lcd_text(uint16_t x, uint16_t y, const char *s,
              uint16_t fg, uint16_t bg, uint8_t scale);

/*
 * Frame streaming API (used by the effects):
 *   lcd_begin_frame()            - select full-screen window, start write
 *   lcd_push(color)  (inline)    - send one pixel (fast path)
 *   lcd_end_frame()              - raise /CS (drains the SPI first)
 */
void lcd_begin_frame(void);
static inline void lcd_push(uint16_t c)
{
    spi_putc(c >> 8);      /* RGB565: high byte first */
    spi_putc(c & 0xFF);
}
void lcd_end_frame(void);
