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
 *
 * The palette is the classic Super Mario NES look (inspired, not a
 * 1:1 copy of Nintendo's art).
 */
#pragma once
#include <stdint.h>

#define LCD_W 240
#define LCD_H 320

/* palette indices — fixed colors 0..20 */
#define C_BLACK        0
#define C_WHITE        1
#define C_RED          2    /* mario red        */
#define C_GREEN        3    /* NES green        */
#define C_BLUE         4    /* overalls blue    */
#define C_YELLOW       5    /* buttons, ? block */
#define C_CYAN         6    /* cloud light      */
#define C_MAGENTA      7    /* cloud shade      */
#define C_GRAY         8
#define C_DARK_GRAY    9    /* hair / shoes brown */
#define C_SKY         10    /* NES sky blue     */
#define C_BROWN       11    /* sienna (ground)  */
#define C_GOLD        12    /* coins            */
#define C_SKIN        13
#define C_ORANGE      14    /* brick base       */
#define C_DARK_GREEN  15    /* grass dark       */
#define C_BRICK_HI    16    /* brick highlight  */
#define C_PIPE        17    /* pipe green       */
#define C_PIPE_DK     18    /* pipe shadow      */
#define C_CASTLE      19    /* castle bricks    */
#define C_CASTLE_DK   20    /* castle shadow    */

/* environment palette: 21..32 sky ramp (deep -> pale),
   33 = far mountains, 34 = near mountains,
   36..47 night sky ramp, 48/49 night hills,
   50..61 sunset ramp (deep purple -> orange), 62..255 = gray ramp */
#define C_SKY_TOP     21
#define C_SKY_HORIZON 32
#define C_MOUNT_FAR   33
#define C_MOUNT_NEAR  34
#define C_NIGHT_TOP   36
#define C_NIGHT_HORIZ 47
#define C_NIGHT_HILL_FAR  48
#define C_NIGHT_HILL_NEAR 49
#define C_SUNSET_TOP      50
#define C_SUNSET_HORIZ    61

void lcd_init(void);
void lcd_clear(uint8_t color);
void lcd_px(int16_t x, int16_t y, uint8_t color);
void lcd_rect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color);
void lcd_sprite(const uint8_t *sprite, uint8_t w, uint8_t h,
                int16_t x, int16_t y, uint8_t transparent);
void lcd_sprite_flip_v(const uint8_t *sprite, uint8_t w, uint8_t h,
                       int16_t x, int16_t y, uint8_t transparent);
void lcd_text(int16_t x, int16_t y, const char *s,
              uint8_t fg, uint8_t bg, uint8_t scale);
void lcd_flush(void);   /* send the framebuffer to the panel */
