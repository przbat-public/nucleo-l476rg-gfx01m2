/*
 * Visual effects for the X-NUCLEO-GFX01M2 demo:
 *
 *   1) PLASMA    — four overlapping sine waves rendered through a
 *                  rotating rainbow palette. The classic demo-scene
 *                  effect, computed with integer math only.
 *   2) STARFIELD — 200 stars flying towards the viewer (perspective
 *                  projection), brighter and bigger as they approach.
 *
 * Performance tricks worth stealing for your own projects:
 *   - a 256-entry sine lookup table instead of sinf() (no FPU needed),
 *   - per-row precomputation: each row of the plasma is turned into
 *     final RGB565 colors BEFORE streaming, so the SPI never waits
 *     for math,
 *   - lcd_begin_frame()/lcd_push()/lcd_end_frame(): one address
 *     window per frame, zero per-pixel command overhead.
 */
#include "fx.h"
#include "st7789.h"
#include <stdint.h>

/* sin(i/256 * 2*PI) * 127, i = 0..255 — one full sine wave. */
static const int16_t SIN_TAB[256] = {
    0, 3, 6, 9, 12, 16, 19, 22, 25, 28, 31, 34,
    37, 40, 43, 46, 49, 51, 54, 57, 60, 63, 65, 68,
    71, 73, 76, 78, 81, 83, 85, 88, 90, 92, 94, 96,
    98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116,
    117, 118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126,
    126, 127, 127, 127, 127, 127, 127, 127, 126, 126, 126, 125,
    125, 124, 123, 122, 122, 121, 120, 118, 117, 116, 115, 113,
    112, 111, 109, 107, 106, 104, 102, 100, 98, 96, 94, 92,
    90, 88, 85, 83, 81, 78, 76, 73, 71, 68, 65, 63,
    60, 57, 54, 51, 49, 46, 43, 40, 37, 34, 31, 28,
    25, 22, 19, 16, 12, 9, 6, 3, 0, -3, -6, -9,
    -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46,
    -49, -51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78,
    -81, -83, -85, -88, -90, -92, -94, -96, -98, -100, -102, -104,
    -106, -107, -109, -111, -112, -113, -115, -116, -117, -118, -120, -121,
    -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127,
    -127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122,
    -122, -121, -120, -118, -117, -116, -115, -113, -112, -111, -109, -107,
    -106, -104, -102, -100, -98, -96, -94, -92, -90, -88, -85, -83,
    -81, -78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51,
    -49, -46, -43, -40, -37, -34, -31, -28, -25, -22, -19, -16,
    -12, -9, -6, -3
};

/* Rainbow palette, rebuilt every frame with a phase offset. */
static uint16_t pal[256];

/* xorshift32 PRNG — good enough for star positions. */
static uint32_t rng_state = 0x9E3779B9u;
static uint32_t rnd(void)
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

/* hue 0..255 -> RGB565 rainbow color (0=red, 42=yellow, 85=green, ...). */
static uint16_t hue565(uint8_t h)
{
    uint16_t r, g, b;
    if (h < 43)       { r = 255; g = (uint16_t)h * 6u;          b = 0; }
    else if (h < 85)  { r = (uint16_t)(85u - h) * 6u;  g = 255; b = 0; }
    else if (h < 128) { r = 0;   g = 255; b = (uint16_t)(h - 85u) * 6u; }
    else if (h < 170) { r = 0;   g = (uint16_t)(170u - h) * 6u; b = 255; }
    else if (h < 213) { r = (uint16_t)(h - 170u) * 6u; g = 0;   b = 255; }
    else              { r = 255; g = 0;   b = (uint16_t)(255u - h) * 6u; }
    return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

static void make_pal(uint8_t rot)
{
    for (uint16_t i = 0; i < 256; i++)
        pal[i] = hue565((uint8_t)(i + rot));
}

/* ------------------------------------------------------------------ */
/* PLASMA                                                              */
/* ------------------------------------------------------------------ */
static uint16_t rowCol[240];   /* one precomputed row of final colors */

static void plasma(uint32_t frames)
{
    for (uint32_t f = 0; f < frames; f++) {
        /* Three time bases drift at different speeds — the pattern
         * never repeats exactly. */
        uint8_t t1 = (uint8_t)(f * 3u);
        uint8_t t2 = (uint8_t)(f * 5u);
        uint8_t t3 = (uint8_t)(f * 7u);
        make_pal((uint8_t)(f * 2u));      /* rotate the palette        */

        lcd_begin_frame();
        for (int y = 0; y < 320; y++) {
            int y2  = y * 2;
            int rowY = SIN_TAB[(y2 + t1) & 255];
            int yb  = (y2 + t3) & 255;
            int yc  = (640 - y2 + t1) & 255;

            /* Precompute the whole row into final colors so that the
             * SPI stream below never stalls on math. */
            for (int x = 0; x < 240; x++) {
                int x2 = x * 2;
                int v = rowY
                      + SIN_TAB[(x2 + t2) & 255]
                      + SIN_TAB[(x2 + yb) & 255]
                      + SIN_TAB[(x2 + yc) & 255];
                rowCol[x] = pal[((v + 512) >> 2) & 255];
            }
            /* Stream the row. */
            for (int x = 0; x < 240; x++)
                lcd_push(rowCol[x]);
        }
        lcd_end_frame();
    }
}

/* ------------------------------------------------------------------ */
/* STARFIELD                                                           */
/* ------------------------------------------------------------------ */
#define NSTARS 200
static struct { int16_t x, y; uint8_t z; } stars[NSTARS];

static void init_stars(void)
{
    for (int i = 0; i < NSTARS; i++) {
        stars[i].x = (int16_t)((rnd() % 480) - 240);
        stars[i].y = (int16_t)((rnd() % 640) - 320);
        stars[i].z = (uint8_t)((rnd() % 254) + 1);
    }
}

static void starfield(uint32_t frames)
{
    for (uint32_t f = 0; f < frames; f++) {
        lcd_fill(0x0000);                  /* clear to black */
        for (int i = 0; i < NSTARS; i++) {
            /* Fly towards the viewer; respawn far away at z<=0. */
            int z = stars[i].z - 3;
            if (z <= 0) {
                stars[i].x = (int16_t)((rnd() % 480) - 240);
                stars[i].y = (int16_t)((rnd() % 640) - 320);
                z = 255;
            }
            stars[i].z = (uint8_t)z;

            /* Perspective projection: screen = pos * f / z + center. */
            int sx = (int)((int32_t)stars[i].x * 96 / z) + 120;
            int sy = (int)((int32_t)stars[i].y * 96 / z) + 160;
            if (sx < 2 || sx > 236 || sy < 2 || sy > 316) continue;

            /* Brightness with a gamma curve — near stars pop. */
            uint32_t b = 255u - (uint32_t)z;
            b = (b * b) >> 8;
            /* Blue-white tint. */
            uint16_t c = (uint16_t)(((b >> 4) << 11) | ((b >> 3) << 5) | (b >> 3));

            uint16_t ux = (uint16_t)sx, uy = (uint16_t)sy;
            if (b > 170)     /* close star: 5x5 px */
                lcd_fill_rect(ux - 2, uy - 2, ux + 2, uy + 2, c);
            else             /* normal star: 3x3 px */
                lcd_fill_rect(ux - 1, uy - 1, ux + 1, uy + 1, c);
        }
    }
}

/* ------------------------------------------------------------------ */
/* The show: plasma, then starfield, forever.                          */
/* ------------------------------------------------------------------ */
void fx_run(void)
{
    init_stars();
    for (;;) {
        plasma(220);     /* ~13 s at ~17 fps */
        starfield(300);  /* ~10 s at ~30 fps */
    }
}
