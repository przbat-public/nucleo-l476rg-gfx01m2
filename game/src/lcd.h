/*
 * lcd.h — display driver with an 8-bit indexed framebuffer.
 *
 * The game draws into a 240x320 byte buffer in RAM (one byte per pixel,
 * an index into a 256-color palette). lcd_flush() converts the buffer
 * to RGB565 and streams it to the ST7789 over SPI.
 *
 * This is the key architectural difference from the effects demo:
 * drawing is instant (RAM), the SPI transfer is one isolated step,
 * and the game never thinks about pixels on a wire.
 */
#pragma once
#include <stdint.h>

#define LCD_W 240
#define LCD_H 320

/* palette indices — the first 16 entries are fixed ANSI-ish colors */
#define C_BLACK        0
#define C_WHITE        1
#define C_RED          2
#define C_GREEN        3
#define C_BLUE         4
#define C_YELLOW       5
#define C_CYAN         6
#define C_MAGENTA      7
#define C_GRAY         8
#define C_DARK_GRAY    9
#define C_SKY         10   /* light blue - sky */
#define C_BROWN       11   /* platforms / ground */
#define C_GOLD        12   /* coins */
#define C_SKIN        13   /* mario */
#define C_ORANGE      14
#define C_DARK_GREEN  15

/* environment palette: 16..27 sky ramp (deep -> pale),
   28 = far mountains, 29 = near mountains, 30..255 = gray ramp */
#define C_SKY_TOP     16
#define C_SKY_HORIZON 27
#define C_MOUNT_FAR   28
#define C_MOUNT_NEAR  29

void lcd_init(void);
void lcd_clear(uint8_t color);
void lcd_px(int16_t x, int16_t y, uint8_t color);
void lcd_rect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color);
void lcd_sprite(const uint8_t *sprite, uint8_t w, uint8_t h,
                int16_t x, int16_t y, uint8_t transparent);
void lcd_text(int16_t x, int16_t y, const char *s,
              uint8_t fg, uint8_t bg, uint8_t scale);
void lcd_flush(void);   /* send the framebuffer to the panel */
