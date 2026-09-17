/*
 * DT022CTFT (ST7789H2) driver — X-NUCLEO-GFX01M2 on NUCLEO-L476RG.
 *
 * The ST7789 is a 240x320 SPI TFT controller. All communication is:
 *   - commands: /CS low, DC low, 1 byte
 *   - data:     /CS low, DC high, N bytes
 * A write is targeted with CASET (columns) + RASET (rows) + RAMWR.
 *
 * The power-up sequence (lcd_init) is deliberately minimal:
 *   hardware reset -> MADCTL -> COLMOD -> SLPOUT -> DISPON.
 * MADCTL=0x08 sets only the BGR color-order bit: the panel expects
 * blue/green/red byte order, and this bit makes red/blue come out
 * right. It also disables mirroring — an earlier version had the MX
 * bit set and the text appeared mirrored on this shield.
 */
#include "st7789.h"
#include "font5x7.h"
#include "regs.h"

/* LCD control pins (see header) */
#define LCD_CS  (1u<<9)   /* PA9  */
#define LCD_DC  (1u<<10)  /* PB10 */
#define LCD_RST (1u<<1)   /* PA1  */

/*
 * cs_set(1) drains the SPI before raising /CS — the ST7789 latches
 * data on the /CS rising edge, so the shift register must be empty.
 */
static void cs_set(uint8_t on) { if (on) { spi_flush(); GPIO_BSRR(0) = LCD_CS; } else GPIO_BRR(0) = LCD_CS; }
static void dc_set(uint8_t on) { if (on) GPIO_BSRR(1) = LCD_DC; else GPIO_BRR(1) = LCD_DC; }

/* Send one command byte (DC=0). */
static void cmd(uint8_t c)
{
    dc_set(0); cs_set(0);
    spi_putc(c);
    cs_set(1);
}

/* Send raw data bytes (DC=1). */
static void data(const uint8_t *d, uint32_t n)
{
    dc_set(1); cs_set(0);
    while (n--) spi_putc(*d++);
    cs_set(1);
}

/* Send the same 16-bit color n times (DC=1). */
static void data16(uint16_t c, uint32_t n)
{
    uint8_t hi = c >> 8, lo = c & 0xFF;
    dc_set(1); cs_set(0);
    while (n--) { spi_putc(hi); spi_putc(lo); }
    cs_set(1);
}

/* Restrict subsequent writes to the rectangle (x0,y0)-(x1,y1). */
static void window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t b[4];
    cmd(0x2A); /* CASET: column range, 2x 16-bit big-endian */
    b[0] = x0 >> 8; b[1] = x0 & 0xFF; b[2] = x1 >> 8; b[3] = x1 & 0xFF;
    data(b, 4);
    cmd(0x2B); /* RASET: row range */
    b[0] = y0 >> 8; b[1] = y0 & 0xFF; b[2] = y1 >> 8; b[3] = y1 & 0xFF;
    data(b, 4);
    cmd(0x2C); /* RAMWR: pixel data follows, auto-incrementing */
}

/* Power up the panel and clear it to black. */
void lcd_init(void)
{
    uint8_t v;

    /* Hardware reset pulse (active low). */
    GPIO_BRR(0)  = LCD_RST; delay_ms(20);
    GPIO_BSRR(0) = LCD_RST; delay_ms(120);

    cmd(0x36); v = 0x08; data(&v, 1); /* MADCTL: BGR color order, no mirror */
    cmd(0x3A); v = 0x55; data(&v, 1); /* COLMOD: 16 bit per pixel (RGB565)  */
    cmd(0x11); delay_ms(120);         /* SLPOUT: exit sleep mode           */
    cmd(0x29); delay_ms(20);          /* DISPON: turn the display on        */

    lcd_fill(0x0000);
}

/* Fill the whole screen with one color. */
void lcd_fill(uint16_t color)
{
    window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    data16(color, (uint32_t)LCD_WIDTH * LCD_HEIGHT);
}

/* Fill a rectangle (coordinates inclusive). */
void lcd_fill_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    window(x0, y0, x1, y1);
    data16(color, (uint32_t)(x1 - x0 + 1) * (y1 - y0 + 1));
}

/* Draw a single pixel. */
void lcd_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    window(x, y, x, y);
    data16(color, 1);
}

/*
 * Streaming API: select the full window once, then push pixels with
 * lcd_push() — no per-pixel command overhead. lcd_end_frame() raises
 * /CS (and drains the SPI). This is how the plasma effect reaches
 * ~17 fps over a 40 MHz SPI link.
 */
void lcd_begin_frame(void)
{
    window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    dc_set(1); cs_set(0);
}

void lcd_end_frame(void)
{
    cs_set(1);
}

/*
 * Text rendering with the built-in 5x7 font (font5x7.c).
 * scale > 1 magnifies each font pixel into a scale x scale block.
 * '\n' advances one line. ASCII 0x20-0x7E only.
 */
void lcd_text(uint16_t x, uint16_t y, const char *s,
              uint16_t fg, uint16_t bg, uint8_t scale)
{
    uint16_t ox = x;

    while (*s) {
        if (*s == '\n') {
            y += 8 * scale;
            x = ox;
            s++;
            continue;
        }
        if (*s < 32 || *s > 126) { s++; continue; }

        const uint8_t *glyph = Font5x7[(uint8_t)(*s - 32)];
        for (uint8_t col = 0; col < 5; col++) {
            uint8_t line = glyph[col];
            for (uint8_t row = 0; row < 7; row++) {
                uint16_t pc = (line & 0x01) ? fg : bg;
                for (uint8_t dx = 0; dx < scale; dx++)
                    for (uint8_t dy = 0; dy < scale; dy++)
                        lcd_pixel(x + col * scale + dx, y + row * scale + dy, pc);
                line >>= 1;
            }
        }
        x += 6 * scale;   /* 5 px glyph + 1 px spacing */
        s++;
    }
}
