# Mini-Mario — a small platformer for NUCLEO-L476RG + X-NUCLEO-GFX01M2

A side-scrolling platformer written from scratch in C — no HAL, no vendor
libraries, ~10 KB of firmware. Built on the layered architecture described
below so that gameplay code stays readable and easy to change.

## Controls (verified with the two-page pin scanner)

| Action | Input |
|---|---|
| Move left / right | joystick LEFT (PB6) / RIGHT (PB0) |
| Jump (hold = higher jump) | the blue USER button **B1** (PC13) |
| Pause / resume | joystick DOWN (PB4) |
| Start / retry / menu | B1 |

Note: PA0 is permanently pulled LOW on this shield and must never be
mapped; the joystick UP contact (PC0) is unused by the game.

## Build & flash

```bash
make            # game.bin
make flash      # st-flash write + reset
make scan       # the joystick pin scanner (diagnostic)
make scanflash
```

## Architecture — where to change what

```
main.c    4 lines: init everything, run the game.
hal.c     THE only file that knows hardware registers
          (clocks, GPIO, SPI). Port the game by rewriting this file.
lcd.c     240x320 framebuffer (8-bit indexed, 75 KB in SRAM1).
          Drawing is RAM access; lcd_flush() streams the frame out.
          API: lcd_px, lcd_rect, lcd_sprite, lcd_text, lcd_flush.
input.c   Joystick. THE PIN MAP is one table at the top —
          change a pin there and nowhere else.
sprites.h Pixel art (Mario, enemy, coins, flag) as palette arrays.
game.c    The game: physics, collisions, camera, states, levels.
scan.c    Joystick pin scanner (development tool).
```

### Game tuning knobs (top of game.c)

| Constant | Meaning | Current |
|---|---|---|
| `GRAVITY` | px/frame² downward | 1 |
| `JUMP_VEL` | initial jump speed | -11 |
| `JUMP_CUT` | rising speed after releasing CENTER (variable jump height) | -4 |
| `MOVE_SPEED` | horizontal speed | 3 |
| `MAX_FALL` | terminal velocity | 12 |

### Adding a level

Levels are ASCII maps in `game.c` (20 rows × 64 columns, 16 px tiles):

```
'.' air   '#' ground   'B' brick   'C' coin
'E' enemy spawn   'F' goal flag   'P' player start
```

Add your map to the `levels[]` array and bump `LEVEL_COUNT`.

### Game states

`TITLE -> PLAYING -> LEVEL_CLEAR -> (next level | WIN)`,
with `GAME_OVER` on losing all lives.
