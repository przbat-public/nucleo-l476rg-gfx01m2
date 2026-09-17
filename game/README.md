# Mini-Mario — a small platformer for NUCLEO-L476RG + X-NUCLEO-GFX01M2

A side-scrolling platformer written from scratch in C — no HAL, no vendor
libraries, ~20 KB of firmware. Built on the layered architecture described
below so that gameplay code stays readable and easy to change.

## Features

- 5 side-scrolling levels (the last one at night), 16 px tile grid,
  a smooth camera and two layers of parallax hills, clouds, bushes
- Mario-feel physics: acceleration, skid, coyote time, jump buffering,
  variable jump height
- Enemies you can stomp — or launch by bumping a block under them
- **Power-ups**: mushroom (G block) grows you into Big Mario — one extra
  hit, and you smash bricks by walking into them; star (S block) gives
  ~6 s of invincibility with a rainbow flash (enemies die on touch)
- Classic extras: blinking invincibility after a hit, coins hidden inside
  every fourth brick, brick debris with gravity, a per-level countdown
  timer with a time bonus, the flag-pole slide + castle ending, stompable
  mushrooms with a collect bounce, drifting clouds, decorative trees and
  rocks along the levels
- Fireballs (joystick DOWN + B1) that bounce along the ground and burn
  enemies; winged flying enemies; a green 1-UP mushroom; a mid-level
  checkpoint pole (respawn point); flag grab height bonus
- Swimmable water ponds, diagonal slopes you can walk up, horizontal
  lifts, a secret Q pipe per level that warps to the bonus coin room
- Visual themes per level: day, sunset (world 2) and night (world 5)
- Saved progress in flash: CONTINUE on the title screen resumes from
  the farthest level reached
- Score, coins, timer and level HUD, plus a high score stored in flash
  (survives power-off)

## Controls (verified with the two-page pin scanner)

| Action | Input |
|---|---|
| Move left / right | joystick LEFT (PB6) / RIGHT (PB0) |
| Jump (hold = higher jump) | the blue USER button **B1** (PC13) |
| Fire a fireball | joystick DOWN + B1 together |
| Pause / resume | joystick DOWN (PB4) alone |
| Enter a secret Q pipe | stand on it, press DOWN |
| Start / retry / menu | B1 |
| Continue from saved level | DOWN on the title screen |

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
sprites.h Pixel art (small & big Mario, mushroom, star, enemy, coins,
          flag) as palette arrays.
game.c    The game: physics, collisions, camera, states, levels,
          power-ups, timer, high score.
scan.c    Joystick pin scanner (development tool).
```

### Game tuning knobs (top of game.c)

| Constant | Meaning | Current |
|---|---|---|
| `MAX_SPEED` | horizontal speed, px/frame | 3 |
| `ACCEL` / `DECEL` | acceleration and friction | 1 / 1 |
| `GRAVITY_RISE` / `GRAVITY_FALL` | gravity going up / down | 1 / 2 |
| `MAX_FALL` | terminal velocity | 12 |
| `JUMP_VEL` | initial jump speed (apex ~78 px: 4-tile shelves reachable) | -12 |
| `JUMP_CUT` | rising speed after releasing B1 (variable jump height) | -5 |
| `COYOTE_FRAMES` / `JUMP_BUFFER` | jump forgiveness, in frames | 4 / 6 |

### Adding a level

Levels are ASCII maps in `game.c` (20 rows × 64 columns, 16 px tiles):

```
'.' air   '#' ground   'B' brick   'C' coin
'E' enemy spawn   'F' goal flag   'P' player start
'?' coin block   'G' mushroom block   'S' star block   'H' 1-UP block
'T' pipe   'Q' secret pipe (warp)   'M' vertical platform   'L' lift
'Y' tree   'R' rock (decorations)   'K' checkpoint   'W' flying enemy
'/' and '\' slopes   '~' water
```

Add your map to the `levels[]` array and bump `LEVEL_COUNT`.
Every fourth brick (deterministic, by tile position) hides a coin.

### Game states

`TITLE -> PLAYING -> CASTLE (flag pole slide + walk in) -> LEVEL_CLEAR ->
(next level | WIN)`, with `GAME_OVER` on losing all lives, `PAUSED` on
joystick DOWN, and `DEAD` for the classic pop-up-and-fall death.
