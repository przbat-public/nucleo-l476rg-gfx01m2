# 05 · The effects

Two classic demo-scene effects, computed with integer math only. All code
is in [`fx.c`](../firmware/main/fx.c).

## Plasma

The plasma effect is the sum of several sine waves sampled at every pixel,
mapped through a color palette:

```
color(x,y) = palette( sin(a·x + t1) + sin(b·y + t2)
                    + sin(c·(x+y) + t3) + sin(d·(x−y) + t4) )
```

### No floats, no FPU

The Cortex-M4F has an FPU, but the demo avoids floats entirely — integer
math is often *faster* on these cores and always more predictable. The
sine function becomes a **256-entry lookup table**:

```c
static const int16_t SIN_TAB[256] = { 0, 3, 6, 9, ... };  /* sin·127 */
```

`SIN_TAB[(x*2 + t) & 255]` gives a full-period sine with automatic
wrap-around — the `& 255` is a free modulo. Four lookups and three adds
per pixel; no division.

### The palette

A hue wheel 0..255 → RGB565, rebuilt every frame with a phase offset:

```c
static uint16_t hue565(uint8_t h) { ... six if-branches ... }
static void make_pal(uint8_t rot) {
    for (uint16_t i = 0; i < 256; i++)
        pal[i] = hue565((uint8_t)(i + rot));
}
```

Rotating the palette while the waves drift makes the pattern evolve
without ever repeating exactly (the three time bases advance at 3, 5 and
7 units per frame — co-prime speeds).

### Performance: precompute the row, stream the pixels

Naive rendering computes-and-sends per pixel — the SPI stalls while the
math runs. The fix is **row precomputation**: turn the whole row into
final RGB565 colors in a buffer, then stream the buffer:

```c
for (int x = 0; x < 240; x++) {
    int v = rowY + SIN_TAB[(x2 + t2) & 255] + ...
    rowCol[x] = pal[((v + 512) >> 2) & 255];     /* math, fast   */
}
for (int x = 0; x < 240; x++)
    lcd_push(rowCol[x]);                          /* pure stream  */
```

The pixel stream then runs at bus speed: 153,600 bytes/frame over a
40 MHz SPI link ≈ 31 ms → ~17 fps with the math hidden in between.

### One window per frame

```c
lcd_begin_frame();      /* CASET + RASET once, CS low, DC high */
... 76800 x lcd_push() ...
lcd_end_frame();        /* drain SPI, CS high                   */
```

`lcd_push` is a `static inline` that writes two bytes — no commands, no
CS toggling, no function call after inlining. That is the entire
difference between a slide show and an animation on an SPI display.

## Starfield

A 3-D point cloud flying at the viewer, drawn with the classic
perspective projection:

```
screen_x = x * f / z + center_x
screen_y = y * f / z + center_y
```

Each star is `(x, y, z)` in a 480×640×256 box around the screen center.
Every frame `z` decreases; at `z ≤ 0` the star respawns far away:

```c
int z = stars[i].z - 3;
if (z <= 0) { stars[i].x = rnd()%480 - 240; ... z = 255; }
int sx = stars[i].x * 96 / z + 120;
int sy = stars[i].y * 96 / z + 160;
```

`f = 96` is the focal length — bigger means a wider field of view.

### Look and feel

- **Brightness**: `b = 255 − z` with a gamma curve `b = b²>>8` — far stars
  are dim, near stars pop. (The first version was linear and the stars
  were invisible on the 2.2" panel.)
- **Size**: close stars (`b > 170`) draw as 5×5 pixel blocks, the rest as
  3×3 — a single pixel was too small to see.
- **Color**: blue-white tint, closer to the warp-through-space look.

The frame is cleared with `lcd_fill(0)` — at 40 MHz that takes ~31 ms, so
the starfield runs at ~30 fps.

## Structure of `fx_run()`

```c
void fx_run(void)
{
    init_stars();
    for (;;) {
        plasma(220);     /* ~13 s */
        starfield(300);  /* ~10 s */
    }
}
```

Swap in your own effects by replacing the loop body — each effect is just
a function that renders `frames` frames.

### Ideas to try

- change the plasma constants (frequencies `a..d`) and watch the pattern
- make the palette a fire gradient (black→red→yellow→white)
- render the starfield *through* the plasma (draw stars after `lcd_fill`
  into the precomputed row buffer)
- add a third scene: Conway's Game of Life, a bouncing ball with trails,
  or a rotating wireframe cube (projection you already have)

---

Next: [06 · Debugging](06-debugging.md)
