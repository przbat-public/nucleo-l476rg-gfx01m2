# 04 · The display

The X-NUCLEO-GFX01M2 carries a **DT022CTFT** panel: 240×320 pixels, RGB565,
driven by an **ST7789H2** controller over SPI. This chapter explains the
wire protocol and the bring-up — the code lives in
[`st7789.c`](../firmware/main/st7789.c).

## SPI in two paragraphs

SPI is a shift-register bus: one clock line (SCK), one data line in each
direction (MOSI/MISO), and a chip select (CS). The master clocks out bits;
both sides shift simultaneously. Our setup:

| Parameter | Value |
|---|---|
| Peripheral | SPI1 |
| Pins | SCK=PA5, MISO=PA6, MOSI=PA7 (alternate function 5) |
| Mode | 0 (CPOL=0, CPHA=0) |
| Word | 8 bits |
| Clock | 40 MHz (APB2 80 MHz, prescaler ÷2) |
| Chip select | software — PA9, toggled manually |

Configuring SPI1 in register land is six lines (see `spi_init()` in
[`main.c`](../firmware/main/main.c)): enable the APB2 clock, set
`MSTR` (master), `BR=000` (÷2), `SSI`+`SSM` (software chip select),
`DS=0b0111` (8-bit), then `SPE`.

Sending one byte:

```c
static inline void spi_putc(uint8_t b)
{
    while (!(SPI1_SR & (1u << 1))) {}   /* wait TXE: buffer empty */
    SPI1_DR8 = b;
}
```

## The ST7789 protocol

The controller distinguishes two kinds of bytes using the **DC** pin:

| DC | Meaning |
|---|---|
| 0 | **command** — the byte is an instruction opcode |
| 1 | **data** — parameter bytes for the previous command |

Every transfer: pull CS low → send bytes → pull CS high. The display
latches on the CS rising edge, so `cs_set(1)` drains the SPI first
(`spi_flush()`).

To draw, you set a **window** (addressable area) and then stream pixels
into it:

```
0x2A CASET   column range  x0_hi x0_lo x1_hi x1_lo   (big-endian 16-bit)
0x2B RASET   row range     y0_hi y0_lo y1_hi y1_lo
0x2C RAMWR   pixels follow, auto-incrementing across the window
```

Each pixel is 2 bytes, RGB565: `RRRRR GGGGGG BBBBB`.

## Bring-up sequence

```c
/* hardware reset pulse (RST = PA1, active low) */
RST = 0; delay_ms(20); RST = 1; delay_ms(120);

cmd(0x36); data(0x08);   /* MADCTL: BGR color order, no mirror  */
cmd(0x3A); data(0x55);   /* COLMOD: 16 bit per pixel            */
cmd(0x11); delay_ms(120);/* SLPOUT: exit sleep                  */
cmd(0x29); delay_ms(20); /* DISPON: display on                  */
```

`MADCTL` is the register you will edit most:

| Bit | Effect |
|---|---|
| `0x08` BGR | swap red/blue (this panel needs it) |
| `0x20` MV  | swap X and Y (rotate 90°) |
| `0x40` MX  | mirror horizontally |
| `0x80` MY  | mirror vertically |

The first version of this project used `MX` and the text came out
mirrored — a one-bit fix.

## Drawing primitives

`st7789.c` provides everything the effects need:

- `lcd_fill(color)` — whole screen
- `lcd_fill_rect(x0,y0,x1,y1,color)` — rectangle
- `lcd_pixel(x,y,color)` — one pixel (sets a 1×1 window)
- `lcd_text(x,y,str,fg,bg,scale)` — 5×7 font, with scaling
- **streaming**: `lcd_begin_frame()` / `lcd_push(color)` /
  `lcd_end_frame()` — one window per frame, then pure pixel data

The streaming API is the secret to the animation speed — see
[05 · The effects](05-effects.md).

## First picture

Flash `firmware/main` and you get the plasma immediately. If you want the
slow path first, comment out `fx_run()` in `main()` and add:

```c
lcd_fill_rect(0, 0, 239, 79, 0xF800);          /* red   */
lcd_fill_rect(0, 80, 239, 159, 0x07E0);        /* green */
lcd_fill_rect(0, 160, 239, 239, 0x001F);       /* blue  */
lcd_text(48, 33, "HELLO!", 0xFFFF, 0x0000, 2); /* text  */
```

Common first-time symptoms: black screen (check shield orientation and
RST pin), mirrored text (`MX` bit), wrong colors (`BGR` bit), washed-out
colors (panel needs an inversion command — not this one).

---

Next: [05 · The effects](05-effects.md)
