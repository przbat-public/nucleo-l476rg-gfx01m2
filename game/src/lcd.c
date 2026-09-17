/*
 * lcd.c — ST7789 display driver with an 8-bit indexed framebuffer.
 *
 * Layers:
 *   framebuffer (this file)  — 240x320 bytes, palette indices
 *   SPI transport (hal.c)    — raw bytes to the ST7789
 *
 * The framebuffer lives in SRAM1 (~75 KB). Drawing is plain memory
 * access; lcd_flush() converts each index through the palette and
 * streams the resulting RGB565 frame in one go.
 */
#include "lcd.h"
#include "hal.h"
#include "font5x7.h"

/* LCD control pins (Arduino-route on the GFX01M2 shield) */
#define PIN_CS  9    /* PA9  */
#define PIN_DC  10   /* PB10 */
#define PIN_RST 1    /* PA1  */

/* 256-entry palette: 16 fixed colors + 240 grays/ramps filled at init */
static uint16_t pal_rgb[256];

/* the framebuffer: 240*320 = 76800 bytes */
static uint8_t fb[LCD_W * LCD_H];

/* ---- low-level ST7789 transport ---- */

static void cs(uint8_t on)
{
    if (on) gpio_set(PORT_A, PIN_CS); else gpio_clear(PORT_A, PIN_CS);
}

static void dc(uint8_t on)
{
    if (on) gpio_set(PORT_B, PIN_DC); else gpio_clear(PORT_B, PIN_DC);
}

static void cmd(uint8_t c)
{
    dc(0); cs(0); spi_write(&c, 1); cs(1);
}

static void data(const uint8_t *d, uint32_t n)
{
    dc(1); cs(0); spi_write(d, n); cs(1);
}

static void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t b[4];
    cmd(0x2A);
    b[0] = x0 >> 8; b[1] = x0; b[2] = x1 >> 8; b[3] = x1;
    data(b, 4);
    cmd(0x2B);
    b[0] = y0 >> 8; b[1] = y0; b[2] = y1 >> 8; b[3] = y1;
    data(b, 4);
    cmd(0x2C);
}

/* ---- palette ---- */

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

static void palette_init(void)
{
    pal_rgb[C_BLACK]      = rgb565(0, 0, 0);
    pal_rgb[C_WHITE]      = rgb565(255, 255, 255);
    pal_rgb[C_RED]        = rgb565(224, 64, 40);    /* mario red        */
    pal_rgb[C_GREEN]      = rgb565(22, 145, 7);     /* NES green        */
    pal_rgb[C_BLUE]       = rgb565(16, 72, 168);    /* overalls blue    */
    pal_rgb[C_YELLOW]     = rgb565(248, 184, 0);    /* buttons/? block  */
    pal_rgb[C_CYAN]       = rgb565(132, 180, 252);  /* cloud light      */
    pal_rgb[C_MAGENTA]    = rgb565(188, 196, 216);  /* cloud shade      */
    pal_rgb[C_GRAY]       = rgb565(176, 176, 176);
    pal_rgb[C_DARK_GRAY]  = rgb565(88, 56, 24);     /* hair/shoes brown */
    pal_rgb[C_SKY]        = rgb565(92, 148, 252);   /* NES sky blue     */
    pal_rgb[C_BROWN]      = rgb565(140, 85, 73);    /* sienna ground    */
    pal_rgb[C_GOLD]       = rgb565(255, 216, 0);    /* coins            */
    pal_rgb[C_SKIN]       = rgb565(240, 176, 136);  /* mario skin       */
    pal_rgb[C_ORANGE]     = rgb565(176, 120, 80);   /* brick base       */
    pal_rgb[C_DARK_GREEN] = rgb565(14, 94, 4);      /* grass dark       */
    pal_rgb[C_BRICK_HI]   = rgb565(208, 152, 112);  /* brick highlight  */
    pal_rgb[C_PIPE]       = rgb565(24, 160, 8);     /* pipe green       */
    pal_rgb[C_PIPE_DK]    = rgb565(12, 96, 4);      /* pipe shadow      */
    pal_rgb[C_CASTLE]     = rgb565(192, 128, 80);   /* castle bricks    */
    pal_rgb[C_CASTLE_DK]  = rgb565(138, 90, 56);    /* castle shadow    */

    /* 12-step sky ramp: deep blue at the top, pale at the horizon */
    for (int i = 0; i < 12; i++) {
        uint8_t r = (uint8_t)(60 + i * 14);    /* 60 .. 214   */
        uint8_t g = (uint8_t)(110 + i * 11);   /* 110 .. 231  */
        uint8_t b = (uint8_t)(255 - i * 2);    /* 255 .. 233  */
        pal_rgb[C_SKY_TOP + i] = rgb565(r, g, b);
    }
    /* classic NES background: rolling GREEN hills, not blue mountains */
    pal_rgb[C_MOUNT_FAR]  = rgb565(108, 180, 60);   /* light hill green */
    pal_rgb[C_MOUNT_NEAR] = rgb565(52, 132, 16);    /* grass green      */

    /* night theme ramp (deep navy -> dark blue) */
    for (int i = 0; i < 12; i++) {
        uint8_t r = (uint8_t)(8 + i * 2);      /* 8 .. 30    */
        uint8_t g = (uint8_t)(16 + i * 4);     /* 16 .. 60   */
        uint8_t b = (uint8_t)(60 + i * 10);    /* 60 .. 170  */
        pal_rgb[C_NIGHT_TOP + i] = rgb565(r, g, b);
    }
    pal_rgb[C_NIGHT_HILL_FAR]  = rgb565(40, 70, 40);
    pal_rgb[C_NIGHT_HILL_NEAR] = rgb565(24, 48, 24);

    /* 50..255: grayscale ramp */
    for (int i = 50; i < 256; i++) {
        uint8_t v = (uint8_t)((i - 50) * 255u / 205u);
        pal_rgb[i] = rgb565(v, v, v);
    }
}

/* ---- public API ---- */

void lcd_init(void)
{
    uint8_t v;

    palette_init();

    gpio_clear(PORT_A, PIN_RST); delay_ms(20);
    gpio_set(PORT_A, PIN_RST);   delay_ms(120);

    cmd(0x36); v = 0x00; data(&v, 1);  /* MADCTL: RGB order, no mirror.
                                           BGR bit CLEAR: with 0x08 the
                                           panel swapped red and blue
                                           (red cap -> blue, yellow coins
                                           -> cyan, blue sky -> orange) */
    cmd(0x3A); v = 0x55; data(&v, 1);  /* COLMOD: 16 bpp        */
    cmd(0x11); delay_ms(120);          /* SLPOUT                */
    cmd(0x29); delay_ms(20);           /* DISPON                */

    lcd_clear(C_BLACK);
    lcd_flush();
}

void lcd_clear(uint8_t color)
{
    for (uint32_t i = 0; i < sizeof(fb); i++) fb[i] = color;
}

void lcd_px(int16_t x, int16_t y, uint8_t color)
{
    if (x < 0 || x >= LCD_W || y < 0 || y >= LCD_H) return;
    fb[(uint32_t)y * LCD_W + x] = color;
}

void lcd_rect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= LCD_W) x1 = LCD_W - 1;
    if (y1 >= LCD_H) y1 = LCD_H - 1;
    for (int16_t y = y0; y <= y1; y++)
        for (int16_t x = x0; x <= x1; x++)
            lcd_px(x, y, color);
}

/*
 * Draw a sprite stored as rows of palette indices
 * (w bytes per row, h rows). Pixels equal to `transparent` are skipped.
 */
void lcd_sprite(const uint8_t *sprite, uint8_t w, uint8_t h,
                int16_t x, int16_t y, uint8_t transparent)
{
    for (uint8_t row = 0; row < h; row++) {
        for (uint8_t col = 0; col < w; col++) {
            uint8_t c = sprite[(uint16_t)row * w + col];
            if (c != transparent)
                lcd_px(x + col, y + row, c);
        }
    }
}

/* like lcd_sprite but vertically mirrored (upside-down enemies) */
void lcd_sprite_flip_v(const uint8_t *sprite, uint8_t w, uint8_t h,
                       int16_t x, int16_t y, uint8_t transparent)
{
    for (uint8_t row = 0; row < h; row++) {
        for (uint8_t col = 0; col < w; col++) {
            uint8_t c = sprite[(uint16_t)(h - 1 - row) * w + col];
            if (c != transparent)
                lcd_px(x + col, y + row, c);
        }
    }
}

void lcd_text(int16_t x, int16_t y, const char *s,
              uint8_t fg, uint8_t bg, uint8_t scale)
{
    int16_t ox = x;
    while (*s) {
        if (*s == '\n') { y += 8 * scale; x = ox; s++; continue; }
        if (*s < 32 || *s > 126) { s++; continue; }
        const uint8_t *g = Font5x7[(uint8_t)(*s - 32)];
        for (uint8_t col = 0; col < 5; col++) {
            uint8_t line = g[col];
            for (uint8_t row = 0; row < 7; row++) {
                uint8_t c = (line & 0x01) ? fg : bg;
                for (uint8_t dx = 0; dx < scale; dx++)
                    for (uint8_t dy = 0; dy < scale; dy++)
                        lcd_px(x + col * scale + dx, y + row * scale + dy, c);
                line >>= 1;
            }
        }
        x += 6 * scale;
        s++;
    }
}

/*
 * Push the whole framebuffer to the panel (~35 ms at 40 MHz).
 * A full repaint every frame — with a scrolling world this is the
 * simple and correct choice (dirty-row tricks leave trails behind
 * moving tiles).
 */
void lcd_flush(void)
{
    volatile uint32_t *spi_sr = (volatile uint32_t *)(0x40013000UL + 0x08);
    volatile uint8_t  *spi_dr = (volatile uint8_t  *)(0x40013000UL + 0x0C);

    set_window(0, 0, LCD_W - 1, LCD_H - 1);
    dc(1); cs(0);

    for (uint32_t i = 0; i < sizeof(fb); i++) {
        uint16_t c = pal_rgb[fb[i]];
        while (!(*spi_sr & (1u << 1))) {}   /* TXE */
        *spi_dr = c >> 8;
        while (!(*spi_sr & (1u << 1))) {}
        *spi_dr = c & 0xFF;
    }
    while (*spi_sr & (1u << 7)) {}          /* drain */
    cs(1);
}
