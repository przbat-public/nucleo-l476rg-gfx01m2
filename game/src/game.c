/*
 * game.c — Mini-Mario.
 *
 * A small side-scrolling platformer:
 *   - 16 px tile grid, 240x320 view, horizontal camera
 *   - player physics: gravity, jumping, axis-separated collision
 *   - walker enemies (stomp them), coins, a goal flag, 5 levels
 *   - power-ups: mushroom (Big Mario), star (temporary invincibility)
 *   - classic extras: hit invincibility, brick debris, coin bricks,
 *     flag-pole slide, per-level countdown timer
 *   - states: TITLE -> PLAYING -> GAME_OVER / WIN
 *
 * The game is pure logic + drawing: every hardware detail is hidden
 * behind hal.h (pins, clocks, SPI) and lcd.h (framebuffer, sprites).
 */
#include "game.h"
#include "hal.h"
#include "lcd.h"
#include "input.h"
#include "sprites.h"
#include <stdint.h>
#include <stdbool.h>

/* ------------------------------ world ------------------------------ */

#define TILE        16
#define LEVEL_COLS  64
#define LEVEL_ROWS  20

/* --- physics (Mario-feel constants) --- */
#define MAX_SPEED     3
#define ACCEL         1    /* px/frame^2 while a direction is held */
#define DECEL         1    /* friction when no direction is held   */
#define SKID_SPEED    2    /* |vx| at which a turn-around skids     */
#define GRAVITY_RISE  1    /* gravity while moving up              */
#define GRAVITY_FALL  2    /* gravity while falling (snappier)     */
#define MAX_FALL      12
#define JUMP_VEL      -11
#define JUMP_CUT      -4    /* rising speed after B1 is released   */
#define COYOTE_FRAMES 4     /* frames of grace after leaving a ledge */
#define JUMP_BUFFER   6     /* frames to buffer a jump before landing */

/* tile kinds */
enum { T_AIR = '.', T_GROUND = '#', T_BRICK = 'B',
       T_COIN = 'C', T_ENEMY = 'E', T_FLAG = 'F', T_PLAYER = 'P',
       T_QB = '?', T_QB_USED = 'U', T_PIPE = 'T',
       T_MUSH = 'G', T_STAR = 'S' };

static const char level1[LEVEL_ROWS][LEVEL_COLS + 1] = {
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "..........................................................F.....",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "....................CCCC................CCC.....................",
    ".....................E.............G...................S........",
    "..........CCC.......BBBB......CCC.......BBB.......CCC...........",
    "................................................................",
    "..........BBB.................BBB.................BBB...........",
    "..P....TT.........................C.C.C.........................",
    "#######TT#######################################################",
    "################################################################",
};

static const char level2[LEVEL_ROWS][LEVEL_COLS + 1] = {
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "............................................................F...",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "........................CCC......CCC............................",
    "...........................G......E..S..........................",
    "........................BBB......BBB............................",
    "................................................................",
    ".................BB...C.......C.................C...............",
    "..P.......TT......................................E..TT.........",
    "##########TT###....######################....########TT#########",
    "###############....######################....###################",
};

static const char level3[LEVEL_ROWS][LEVEL_COLS + 1] = {
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    ".............................................................F..",
    "................................................................",
    "................................................................",
    "..................BB............................................",
    "............CCB............C....................................",
    "..................................CCC.............CCC...........",
    "............BB..................................................",
    "................G.........CCC.....BBB.....CSC.....BBB...........",
    "...........................................E...................",
    "......BB..................BBB.............BBB...................",
    "........................................C..............C........",
    ".P.ETT..........E..........TT...................................",
    "####TT#####...#######...###TT###################################",
    "###########...#######...########################################",
};

static const char level4[LEVEL_ROWS][LEVEL_COLS + 1] = {
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "..............................................................F.",
    "................................................................",
    ".............CCC............................CCC.................",
    "........................CCC.....................................",
    ".............BBB............................BBB.................",
    "........CCC.............BBB.............CCC.....................",
    "....................CCC.....CCC..........E..............E.......",
    "........BBB......G....E...............S.BBB.............BBB.....",
    "....................BBB.....BBB.................................",
    "................................................................",
    "................................................................",
    ".PE...TT.........................CC....CCC................TT....",
    "######TT####....###############....##############....#####TT####",
    "############....###############....##############....###########",
};

static const char level5[LEVEL_ROWS][LEVEL_COLS + 1] = {
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    ".............................................................F..",
    "................................................................",
    "................................................................",
    "............CCC...........................CCC...................",
    "...........................................ES...................",
    "............BBB...CCC...............CCC...BBB...................",
    "......CCC.........G.....CCC.....................................",
    "..................BBB...............BBB.........................",
    "......BBB...............BMB....M...............M........BBB.....",
    "................................................................",
    "......................C.........................................",
    ".PE...........................CC........C...........C.......C...",
    "############....##############....############....##############",
    "############....##############....############....##############",
};

static const char *const levels[] = { &level1[0][0], &level2[0][0],
                                      &level3[0][0], &level4[0][0],
                                      &level5[0][0] };
#define LEVEL_COUNT 5

/* mutable copy of the current level (coins get removed, etc.) */
static uint8_t grid[LEVEL_ROWS][LEVEL_COLS];

/* ------------------------------ entities -------------------------- */

typedef struct {
    int  x, y;          /* top-left, pixels */
    int  vx, vy;        /* px per frame */
    bool on_ground;
    bool facing_right;
    bool big;           /* mushroom power-up: two tiles tall */
} player_t;

typedef struct {
    int  x, y;
    int  vx, vy;       /* vy: only used while flipped by a block bump */
    bool alive;
    bool flipped;      /* launched by a block hit: tumbles off screen */
} enemy_t;

/* mushroom / star power-up dropped out of a G / S block */
typedef struct {
    int  x, y;
    int  vx, vy;
    int  rising;       /* frames left while emerging from the block */
    bool active;
    bool star;         /* true: bouncing star, false: walking mushroom */
} powerup_t;

static player_t  player;
static enemy_t   enemies[4];
static int       enemy_count;
static powerup_t powup;
static uint32_t  frame;       /* frame counter (animations) */

/* classic hit invincibility: a few frames of blinking after damage */
static int iframes;
/* star power-up: kills on touch + palette flash while active */
static int star_timer;

/* ------------------------------ particles ------------------------- */

#define MAX_PARTS 32
typedef struct {
    int16_t x, y;        /* screen space */
    int8_t  vx, vy;
    uint8_t life;
    uint8_t color;
} particle_t;

static particle_t parts[MAX_PARTS];

/* coin popping out of a ? block (screen space) */
typedef struct { int x, y, vy; uint8_t life; } pop_coin_t;
static pop_coin_t pop;

/* Spawn n short-lived pixels flying out of (x, y). */
static void spawn_burst(int x, int y, uint8_t color, int n)
{
    int spawned = 0;
    for (int i = 0; i < MAX_PARTS && spawned < n; i++) {
        if (parts[i].life != 0) continue;
        parts[i].x = (int16_t)x;
        parts[i].y = (int16_t)y;
        parts[i].vx = (int8_t)((frame * 7 + i * 13) % 7 - 3);   /* -3..3 */
        parts[i].vy = (int8_t)(-5 - ((frame + i) % 3));
        parts[i].life = 12 + (uint8_t)((frame + i * 5) % 6);
        parts[i].color = color;
        spawned++;
    }
}

static void update_particles(void)
{
    for (int i = 0; i < MAX_PARTS; i++) {
        if (parts[i].life == 0) continue;
        parts[i].x += parts[i].vx;
        parts[i].y += parts[i].vy;
        parts[i].vy += 1;                      /* gravity */
        parts[i].life--;
    }
}

static void render_particles(void)
{
    for (int i = 0; i < MAX_PARTS; i++) {
        if (parts[i].life == 0) continue;
        lcd_px(parts[i].x, parts[i].y, parts[i].color);
    }
    /* coin popping out of a ? block */
    if (pop.life > 0) {
        int cf = (int)((frame >> 2) & 3);
        lcd_sprite(coin_sprite[cf][0], COIN_W, COIN_H,
                   pop.x, pop.y, SPR_TRANSPARENT);
    }
}

/* screen shake: shake_timer frames of a +-2 px jitter */
static int shake_timer;
static int shake_x;

/* ------------------------------ moving platforms ------------------ */

#define MAX_MOVERS 6
#define MOV_W      48
#define MOV_H      12
typedef struct { int x, y; int base_y; int amp; int phase; int dy; } mover_t;
static mover_t movers[MAX_MOVERS];
static int mover_count;
static int ride = -1;    /* mover the player is standing on */

/* 64-entry sine * 31 */
static const int8_t MOV_SIN[64] = {
     0,  3,  6,  9, 12, 15, 18, 20, 23, 25, 27, 29, 30, 31, 31, 31,
    31, 31, 31, 30, 29, 27, 25, 23, 20, 18, 15, 12,  9,  6,  3,  0,
     0, -3, -6, -9,-12,-15,-18,-20,-23,-25,-27,-29,-30,-31,-31,-31,
   -31,-31,-31,-30,-29,-27,-25,-23,-20,-18,-15,-12, -9, -6, -3,  0,
};

/* ------------------------------ game state ------------------------ */

typedef enum { S_TITLE, S_PLAYING, S_LEVEL_CLEAR, S_GAME_OVER, S_WIN,
               S_PAUSED, S_DEAD, S_INTRO, S_CASTLE } state_t;

static state_t state;
static int     level_idx;
static int     score;
static int     coins;
static int     lives;
static int     high_score;  /* best score (persisted in flash) */
static int     cam_x;
static int     clear_timer;  /* frames left on the LEVEL CLEAR screen */
static int     dead_timer;   /* frames left on the death screen */
static int     intro_timer;  /* frames left on the WORLD intro card */
static int     flash_timer;  /* white flash when the flag is reached */
static int     time_left;    /* countdown timer, in seconds */
static int     time_tick;    /* frame accumulator for the timer */
static int     bonus;        /* time bonus awarded at the level clear */

/* ------------------------------ prototypes ------------------------ */

static bool solid_at(int tx, int ty);
static bool box_hits(int x, int y, int w, int h);
static bool overlap(int ax, int ay, int aw, int ah,
                    int bx, int by, int bw, int bh);
static void player_die(void);
/* castle ending: grab the flag pole, slide down, walk into the castle */
static int  flag_tx;
static bool auto_right;
static bool entering_castle;
static bool flag_done;    /* the flag was already grabbed this level */
static int  castle_phase;   /* 0 = sliding down the pole,
                               1 = walking to the door,
                               2 = sliding into the castle */

static void start_level(int n);
static void draw_mario(int x, int y, bool facing_right);
static void flip_enemies_on(int tx, int hty);
static void break_brick(int tx, int ty);

/* ------------------------------ level loading --------------------- */

static bool solid_tile(uint8_t t)
{
    return t == T_GROUND || t == T_BRICK || t == T_PIPE ||
           t == T_QB || t == T_QB_USED || t == T_MUSH || t == T_STAR;
}

static bool solid_at(int tx, int ty)
{
    if (ty < 0)  return true;                      /* ceiling         */
    if (tx < 0 || tx >= LEVEL_COLS) return false;  /* sides: fall off */
    if (ty >= LEVEL_ROWS) return false;            /* below: fall     */
    return solid_tile(grid[ty][tx]);
}

/* Is there a solid tile anywhere in the rectangle? */
static bool box_hits(int x, int y, int w, int h)
{
    int x0 = x / TILE, x1 = (x + w - 1) / TILE;
    int y0 = y / TILE, y1 = (y + h - 1) / TILE;
    for (int ty = y0; ty <= y1; ty++)
        for (int tx = x0; tx <= x1; tx++)
            if (solid_at(tx, ty))
                return true;
    return false;
}

static bool overlap(int ax, int ay, int aw, int ah,
                    int bx, int by, int bw, int bh)
{
    return ax < bx + bw && ax + aw > bx &&
           ay < by + bh && ay + ah > by;
}

/* current player height: two tiles tall while Big Mario */
static int mario_h(void)
{
    return player.big ? MARIO_BIG_H : MARIO_H;
}

/* four brick shards with real gravity, arcing out of the broken block */
static void spawn_shards(int x, int y)
{
    static const int8_t svx[4] = { -2, -1, 1, 2 };
    static const int8_t svy[4] = { -5, -6, -6, -5 };
    int placed = 0;
    for (int i = 0; i < MAX_PARTS && placed < 4; i++) {
        if (parts[i].life != 0) continue;
        parts[i].x = (int16_t)(x + svx[placed] * 3);
        parts[i].y = (int16_t)(y - 4);
        parts[i].vx = svx[placed];
        parts[i].vy = svy[placed];
        parts[i].life = 26;
        parts[i].color = C_ORANGE;
        placed++;
    }
}

/* break a brick: some bricks hide a coin, others shatter into shards */
static void break_brick(int tx, int ty)
{
    grid[ty][tx] = T_AIR;
    score += 10;
    /* deterministic coin bricks: about every fourth brick */
    if (((tx * 7 + ty * 13) & 3) == 0) {
        coins++;
        pop.x = tx * TILE + 4 - cam_x;
        pop.y = ty * TILE - 10;
        pop.vy = -7;
        pop.life = 30;
    } else {
        spawn_shards(tx * TILE + 8 - cam_x, ty * TILE + 8);
    }
    flip_enemies_on(tx, ty);
}

/* a G / S block is bumped: the power-up rises out of the block top */
static void spawn_powerup(int tx, int hty, bool is_star)
{
    powup.x = tx * TILE + 2;
    powup.y = hty * TILE + 2;       /* hidden inside the block first */
    powup.vx = is_star ? 2 : 1;
    powup.vy = 0;
    powup.rising = 14;              /* frames spent emerging */
    powup.star = is_star;
    powup.active = true;
}

static void update_powerup(void)
{
    if (!powup.active) return;

    if (powup.rising > 0) {         /* emerge from the block */
        powup.rising--;
        powup.y--;
        return;
    }

    if (powup.star) {
        /* star: hops along the ground, bouncing off walls */
        powup.vy += 1;
        powup.x += powup.vx;
        powup.y += powup.vy;
        int fx = powup.x + (powup.vx > 0 ? 12 : 0);
        if (solid_at(fx / TILE, powup.y / TILE) ||
            solid_at(fx / TILE, (powup.y + 11) / TILE))
            powup.vx = -powup.vx;
        if (powup.vy >= 0 &&
            box_hits(powup.x, powup.y + 11, 12, 2)) {
            powup.vy = -9;          /* bounce */
        }
    } else {
        /* mushroom: walks like a classic enemy, falls off ledges */
        powup.vy += GRAVITY_FALL;
        if (powup.vy > MAX_FALL) powup.vy = MAX_FALL;
        powup.x += powup.vx;
        int fx = powup.x + (powup.vx > 0 ? 12 : 0);
        if (solid_at(fx / TILE, powup.y / TILE) ||
            solid_at(fx / TILE, (powup.y + 11) / TILE))
            powup.vx = -powup.vx;
        int ny = powup.y + powup.vy;
        if (box_hits(powup.x + 1, ny + 11, 10, 1)) {
            powup.y = (ny + 12) / TILE * TILE - 12;   /* snap on top */
            powup.vy = 0;
        } else {
            powup.y = ny;
        }
    }

    if (powup.y > LCD_H + 32) powup.active = false;

    /* player pickup */
    if (overlap(player.x, player.y, MARIO_W, mario_h(),
                powup.x, powup.y, 12, 12)) {
        powup.active = false;
        score += 100;
        if (powup.star) {
            star_timer = 100;       /* ~6 s of touch-death invincibility */
        } else {
            player.big = true;      /* grow! */
            /* keep the feet planted: raise the top by the extra height */
            player.y -= MARIO_BIG_H - MARIO_H;
        }
        spawn_burst(powup.x - cam_x, powup.y, C_GOLD, 10);
    }
}

static void start_level(int n)
{
    const char *src = levels[n];
    enemy_count = 0;
    mover_count = 0;
    ride = -1;

    for (int ty = 0; ty < LEVEL_ROWS; ty++) {
        for (int tx = 0; tx < LEVEL_COLS; tx++) {
            uint8_t t = (uint8_t)src[ty * (LEVEL_COLS + 1) + tx];
            switch (t) {
            case T_PLAYER:
                player.x = tx * TILE + 2;
                player.y = ty * TILE;              /* h == TILE: feet touch next row */
                t = T_AIR;
                break;
            case T_ENEMY:
                if (enemy_count < 4) {
                    enemies[enemy_count].x = tx * TILE + 2;
                    enemies[enemy_count].y = ty * TILE + (TILE - ENEMY_H);
                    enemies[enemy_count].vx = -1;
                    enemies[enemy_count].vy = 0;
                    enemies[enemy_count].alive = true;
                    enemies[enemy_count].flipped = false;
                    enemy_count++;
                }
                t = T_AIR;
                break;
            case 'M':   /* moving platform anchor */
                if (mover_count < MAX_MOVERS) {
                    movers[mover_count].x = tx * TILE;
                    movers[mover_count].y = ty * TILE;
                    movers[mover_count].base_y = ty * TILE;
                    movers[mover_count].amp = 26;
                    movers[mover_count].phase = 0;
                    movers[mover_count].dy = 0;
                    mover_count++;
                }
                t = T_AIR;
                break;
            default:
                break;
            }
            grid[ty][tx] = t;
        }
    }

    player.vx = player.vy = 0;
    player.on_ground = false;
    player.facing_right = true;
    player.big = false;
    iframes = 0;
    star_timer = 0;
    powup.active = false;
    time_left = 300;               /* classic 5-minute countdown */
    time_tick = 0;
    bonus = 0;
    cam_x = 0;
    auto_right = false;
    entering_castle = false;
    castle_phase = 0;
    flag_done = false;
    intro_timer = 40;              /* "WORLD N" card, then play */
    state = S_INTRO;
}

/* moving platforms: swing up/down on a sine */
static void update_movers(void)
{
    for (int i = 0; i < mover_count; i++) {
        mover_t *m = &movers[i];
        int prev = m->y;
        m->phase = (m->phase + 1) & 63;
        m->y = m->base_y + m->amp * MOV_SIN[m->phase] / 31;
        m->dy = m->y - prev;
    }
}

/* ------------------------------ physics --------------------------- */

/* --- player physics state (coyote time, jump buffering, skid) --- */
static int  coyote;        /* frames left in which a jump is still legal */
static int  jump_buffer;   /* frames left to auto-jump after landing     */
static bool skidding;
static bool prev_center;   /* edge detection for the B1 button           */

static void update_player(void)
{
    bool left  = input_held(DIR_LEFT) && !auto_right;
    bool right = input_held(DIR_RIGHT) || auto_right;
    bool center = input_held(DIR_CENTER);

    /* --- horizontal: accelerate / decelerate / skid --- */
    if (right && !left) {
        if (player.vx < 0 && player.vx <= -SKID_SPEED)
            skidding = true;                 /* turn-around skid */
        player.vx += ACCEL;
        if (player.vx > MAX_SPEED) player.vx = MAX_SPEED;
    } else if (left && !right) {
        if (player.vx > 0 && player.vx >= SKID_SPEED)
            skidding = true;
        player.vx -= ACCEL;
        if (player.vx < -MAX_SPEED) player.vx = -MAX_SPEED;
    } else {
        /* no input: friction */
        if (player.vx > 0) { player.vx -= DECEL; if (player.vx < 0) player.vx = 0; }
        if (player.vx < 0) { player.vx += DECEL; if (player.vx > 0) player.vx = 0; }
        skidding = false;
    }
    if (player.vx > 0) player.facing_right = true;
    if (player.vx < 0) player.facing_right = false;

    /* running / skidding dust */
    if (player.on_ground && (player.vx == MAX_SPEED || player.vx == -MAX_SPEED
                             || skidding) && (frame & 5) == 0) {
        spawn_burst(player.x + (player.vx > 0 ? 0 : MARIO_W) - cam_x,
                    player.y + mario_h() - 2, C_GRAY, 1);
    }

    /* --- jumping: coyote time + buffering --- */
    if (center && !prev_center)
        jump_buffer = JUMP_BUFFER;           /* arm the buffer */
    prev_center = center;

    if (player.on_ground) coyote = COYOTE_FRAMES;
    else if (coyote > 0) coyote--;

    if ((center || jump_buffer > 0) && (player.on_ground || coyote > 0)) {
        player.vy = JUMP_VEL;
        player.on_ground = false;
        coyote = 0;
        jump_buffer = 0;
    }
    if (jump_buffer > 0) jump_buffer--;

    /* --- gravity: rises gently, falls snappy --- */
    if (player.vy < 0) {
        player.vy += GRAVITY_RISE;
        /* variable jump height: releasing B1 cuts the rise short */
        if (player.vy < JUMP_CUT && !center)
            player.vy = JUMP_CUT;
    } else {
        player.vy += GRAVITY_FALL;
    }
    if (player.vy > MAX_FALL) player.vy = MAX_FALL;

    /* ride the moving platform we stand on */
    if (ride >= 0) {
        mover_t *m = &movers[ride];
        if (overlap(player.x, player.y + mario_h() - 2, MARIO_W, 2,
                    m->x, m->y - 4, MOV_W, 10))
            player.y += m->dy;
        else
            ride = -1;
    }

    /* --- vertical move with collision --- */
    bool was_ground = player.on_ground;
    if (player.vy >= 0) {
        /* falling: check the feet line */
        int ny = player.y + player.vy;
        int feet = ny + mario_h();
        bool landed = false;
        if (box_hits(player.x + 1, feet - 1, MARIO_W - 2, 1)) {
            player.y = (feet / TILE) * TILE - mario_h();  /* snap on top */
            if (!was_ground && player.vy >= 5) {
                /* landing dust */
                spawn_burst(player.x + 2 - cam_x, player.y + mario_h() - 2,
                            C_GRAY, 4);
                spawn_burst(player.x + MARIO_W - 2 - cam_x,
                            player.y + mario_h() - 2, C_GRAY, 4);
            }
            player.vy = 0;
            player.on_ground = true;
            landed = true;
        }
        if (!landed) {
            /* land on a moving platform? */
            for (int i = 0; i < mover_count; i++) {
                mover_t *m = &movers[i];
                if (overlap(player.x, player.y + mario_h() - 2, MARIO_W, 3,
                            m->x, m->y - 2, MOV_W, 8)) {
                    player.y = m->y - mario_h();
                    player.vy = 0;
                    player.on_ground = true;
                    ride = i;
                    landed = true;
                    break;
                }
            }
        }
        if (!landed) {
            player.y = ny;
            player.on_ground = false;
        }
    } else {
        /* rising: check the head line */
        int ny = player.y + player.vy;
        if (box_hits(player.x + 1, ny, MARIO_W - 2, 1)) {
            player.y = (ny / TILE + 1) * TILE;           /* snap below */
            player.vy = 0;
            /* bump blocks from below */
            int hty = ny / TILE;
            int hx0 = (player.x + 1) / TILE;
            int hx1 = (player.x + MARIO_W - 2) / TILE;
            for (int tx = hx0; tx <= hx1; tx++) {
                if (tx < 0 || tx >= LEVEL_COLS || hty < 0 || hty >= LEVEL_ROWS)
                    continue;
                if (grid[hty][tx] == T_QB) {
                    /* ? block -> coin pops out */
                    grid[hty][tx] = T_QB_USED;
                    score += 10;
                    coins++;
                    spawn_burst(tx * TILE + 8 - cam_x, hty * TILE, C_GOLD, 6);
                    pop.x = tx * TILE + 4 - cam_x;
                    pop.y = hty * TILE - 10;
                    pop.vy = -7;
                    pop.life = 30;
                    flip_enemies_on(tx, hty);
                } else if (grid[hty][tx] == T_MUSH) {
                    /* mushroom block -> mushroom power-up pops out */
                    grid[hty][tx] = T_QB_USED;
                    spawn_powerup(tx, hty, false);
                    flip_enemies_on(tx, hty);
                } else if (grid[hty][tx] == T_STAR) {
                    /* star block -> star power-up pops out */
                    grid[hty][tx] = T_QB_USED;
                    spawn_powerup(tx, hty, true);
                    flip_enemies_on(tx, hty);
                } else if (grid[hty][tx] == T_BRICK) {
                    /* brick: shards or a hidden coin */
                    break_brick(tx, hty);
                }
            }
        } else {
            player.y = ny;
        }
    }

    /* --- horizontal move with collision --- */
    if (player.vx != 0) {
        int nx = player.x + player.vx;
        if (box_hits(nx, player.y + 1, MARIO_W, mario_h() - 2)) {
            if (player.big) {
                /* Big Mario plows through bricks without jumping */
                int lead = player.vx > 0 ? nx + MARIO_W - 1 : nx;
                bool smashed = false;
                int ttx = lead / TILE;
                int ty0 = (player.y + 1) / TILE;
                int ty1 = (player.y + mario_h() - 2) / TILE;
                if (ty1 >= LEVEL_ROWS) ty1 = LEVEL_ROWS - 1;
                if (ttx >= 0 && ttx < LEVEL_COLS) {
                    for (int ty = ty0; ty <= ty1; ty++)
                        if (ty >= 0 && grid[ty][ttx] == T_BRICK) {
                            break_brick(ttx, ty);
                            smashed = true;
                        }
                }
                if (smashed) {
                    player.x = nx;          /* keep walking */
                } else {
                    /* real wall: push against it */
                    if (player.vx > 0) player.x = (nx / TILE) * TILE - MARIO_W;
                    else               player.x = (nx / TILE + 1) * TILE;
                    player.vx = 0;
                }
            } else {
                /* push against the wall */
                if (player.vx > 0) player.x = (nx / TILE) * TILE - MARIO_W;
                else               player.x = (nx / TILE + 1) * TILE;
                player.vx = 0;
            }
        } else {
            player.x = nx;
        }
    }

    /* --- collect coins the player overlaps --- */
    int x0 = player.x / TILE, x1 = (player.x + MARIO_W - 1) / TILE;
    int y0 = player.y / TILE, y1 = (player.y + mario_h() - 1) / TILE;
    for (int ty = y0; ty <= y1; ty++)
        for (int tx = x0; tx <= x1; tx++) {
            if (tx < 0 || tx >= LEVEL_COLS || ty < 0 || ty >= LEVEL_ROWS) continue;
            if (grid[ty][tx] == T_COIN) {
                grid[ty][tx] = T_AIR;
                score += 10;
                coins++;
                /* 100 coins = one extra life (classic) */
                if (coins >= 100) {
                    coins -= 100;
                    if (lives < 9) lives++;
                    spawn_burst(tx * TILE + 8 - cam_x, ty * TILE + 8,
                                C_GOLD, 12);
                } else {
                    spawn_burst(tx * TILE + 8 - cam_x, ty * TILE + 8,
                                C_GOLD, 6);
                }
            }
        }

    /* --- reached the goal flag? -> grab the pole and slide down --- */
    int ptx = (player.x + MARIO_W / 2) / TILE;
    if (!flag_done)
        for (int ty = 0; ty < LEVEL_ROWS; ty++) {
            if (grid[ty][ptx] == T_FLAG) {
                score += 100;
                flag_tx = ptx;
                flag_done = true;
                player.x = ptx * TILE + 2;     /* grab the pole */
                player.vx = 0;
                player.vy = 0;
                player.facing_right = true;
                auto_right = false;
                entering_castle = false;
                castle_phase = 0;              /* slide down first */
                state = S_CASTLE;
                return;
            }
        }

    /* --- fell into a pit --- */
    if (player.y > LCD_H + 16)
        player_die();
}

/* A block was hit from below: any enemy standing on it is launched
 * up, flipped, and tumbles off the screen (classic Mario). */
static void flip_enemies_on(int tx, int hty)
{
    for (int i = 0; i < enemy_count; i++) {
        enemy_t *e = &enemies[i];
        if (!e->alive || e->flipped) continue;
        if (e->x + ENEMY_W > tx * TILE && e->x < tx * TILE + TILE &&
            e->y + ENEMY_H >= hty * TILE - 4 &&
            e->y + ENEMY_H <= hty * TILE + 10) {
            e->flipped = true;
            e->vy = -9;
            score += 50;
        }
    }
}

static void update_enemies(void)
{
    for (int i = 0; i < enemy_count; i++) {
        enemy_t *e = &enemies[i];
        if (!e->alive) continue;

        /* flipped by a block bump: tumble off the screen */
        if (e->flipped) {
            e->vy += GRAVITY_FALL;
            e->y += e->vy;
            if (e->y > LCD_H + 32) e->alive = false;
            continue;
        }

        /* wall in front -> turn around */
        int fx = e->x + (e->vx > 0 ? ENEMY_W : -1);
        if (solid_at(fx / TILE, e->y / TILE) ||
            solid_at(fx / TILE, (e->y + ENEMY_H - 1) / TILE)) {
            e->vx = -e->vx;
        } else {
            /* no ground under the front foot -> turn (platform edge) */
            int foot_x = e->x + (e->vx > 0 ? ENEMY_W : -1);
            int foot_y = (e->y + ENEMY_H + 2) / TILE;
            if (!solid_at(foot_x / TILE, foot_y))
                e->vx = -e->vx;
        }
        e->x += e->vx;

        /* collide with the player */
        if (overlap(player.x, player.y, MARIO_W, mario_h(),
                    e->x, e->y, ENEMY_W, ENEMY_H)) {
            if (star_timer > 0) {
                /* star power: touching an enemy destroys it */
                e->alive = false;
                score += 100;
                spawn_burst(e->x + ENEMY_W / 2 - cam_x,
                            e->y + ENEMY_H / 2, C_GOLD, 8);
            } else if (player.vy > 0 &&
                       (player.y + mario_h() - e->y) < 10) {
                /* stomp! */
                e->alive = false;
                player.vy = -8;
                score += 50;
                spawn_burst(e->x + ENEMY_W / 2 - cam_x,
                            e->y + ENEMY_H / 2, C_ORANGE, 8);
            } else if (iframes > 0) {
                /* invincibility frames: pass straight through */
            } else if (player.big) {
                /* Big Mario survives one hit: shrink, don't die */
                player.big = false;
                /* keep the feet planted: drop the top back down */
                player.y += MARIO_BIG_H - MARIO_H;
                iframes = 40;       /* ~2 s of blinking invincibility */
                player.vy = -6;
                spawn_burst(e->x + ENEMY_W / 2 - cam_x,
                            e->y + ENEMY_H / 2, C_YELLOW, 10);
            } else {
                player_die();
                return;
            }
        }
    }
}

static void player_die(void)
{
    shake_timer = 18;              /* screen shake during the death    */
    player.vy = -9;                /* classic Mario death: pop up,     */
    player.vx = 0;                 /* then fall off the screen         */
    player.on_ground = false;
    iframes = 0;
    star_timer = 0;
    dead_timer = 70;
    state = S_DEAD;
}

/* ------------------------------ camera ---------------------------- */

static void update_camera(void)
{
    int target = player.x - 100;
    if (target < 0) target = 0;
    if (target > LEVEL_COLS * TILE - LCD_W) target = LEVEL_COLS * TILE - LCD_W;
    cam_x = target;
}

/* ------------------------------ rendering ------------------------- */

static void fmt_int(char *buf, int v)
{
    char tmp[8];
    int i = 0;
    if (v == 0) tmp[i++] = '0';
    while (v > 0) { tmp[i++] = (char)('0' + v % 10); v /= 10; }
    while (i > 0) *buf++ = tmp[--i];
    *buf = '\0';
}

/* like fmt_int but zero-padded to `width` digits (classic score) */
static void fmt_int_pad(char *buf, int v, int width)
{
    char tmp[8];
    int i = 0;
    if (v == 0) tmp[i++] = '0';
    while (v > 0) { tmp[i++] = (char)('0' + v % 10); v /= 10; }
    while (i < width) tmp[i++] = '0';
    while (i > 0) *buf++ = tmp[--i];
    *buf = '\0';
}

static void draw_tile(int sx, int sy, int tx, uint8_t t)
{
    int ty = sy / TILE;

    switch (t) {
    case T_GROUND:
        lcd_rect(sx, sy, sx + TILE - 1, sy + TILE - 1, C_BROWN);
        lcd_rect(sx, sy, sx + TILE - 1, sy + 3, C_DARK_GREEN);   /* grass */
        lcd_rect(sx, sy, sx + TILE - 1, sy, C_GREEN);            /* highlight */
        lcd_rect(sx, sy + 4, sx + TILE - 1, sy + 4, C_DARK_GRAY);/* NES dark edge */
        /* deterministic dirt speckles */
        if (((tx * 7 + ty * 13) & 3) == 0) lcd_px(sx + 3, sy + 9, C_DARK_GRAY);
        if (((tx * 5 + ty * 11) & 3) == 0) lcd_px(sx + 10, sy + 13, C_DARK_GRAY);
        break;

    case T_BRICK:
        lcd_rect(sx, sy, sx + TILE - 1, sy + TILE - 1, C_ORANGE);
        lcd_rect(sx, sy, sx + TILE - 1, sy, C_BRICK_HI);         /* top light */
        lcd_rect(sx, sy + TILE - 1, sx + TILE - 1, sy + TILE - 1, C_BLACK);
        lcd_rect(sx, sy + 5, sx + TILE - 1, sy + 5, C_BLACK);    /* NES mortar */
        lcd_rect(sx, sy + 10, sx + TILE - 1, sy + 10, C_BLACK);
        lcd_rect(sx + 5, sy, sx + 5, sy + 4, C_BLACK);
        lcd_rect(sx + 11, sy + 6, sx + 11, sy + 9, C_BLACK);
        lcd_rect(sx + 2, sy + 11, sx + 2, sy + 14, C_BLACK);
        break;

    case T_COIN: {
        int f = (int)((frame >> 2) & 3);   /* 4-frame rotation */
        lcd_sprite(coin_sprite[f][0], COIN_W, COIN_H,
                   sx + 4, sy + 4, SPR_TRANSPARENT);
        break;
    }

    case T_QB:
        /* classic yellow ? block */
        lcd_rect(sx, sy, sx + TILE - 1, sy + TILE - 1, C_YELLOW);
        lcd_rect(sx, sy, sx + TILE - 1, sy, C_BRICK_HI);
        lcd_rect(sx, sy + TILE - 1, sx + TILE - 1, sy + TILE - 1, C_DARK_GRAY);
        lcd_px(sx + 1, sy + 1, C_BROWN);   lcd_px(sx + 14, sy + 1, C_BROWN);
        lcd_px(sx + 1, sy + 14, C_BROWN);  lcd_px(sx + 14, sy + 14, C_BROWN);
        lcd_text(sx + 5, sy + 4, "?", C_BROWN, C_YELLOW, 1);
        break;

    case T_QB_USED:
        /* used block: brown, no mark */
        lcd_rect(sx, sy, sx + TILE - 1, sy + TILE - 1, C_BROWN);
        lcd_rect(sx, sy, sx + TILE - 1, sy, C_BRICK_HI);
        lcd_rect(sx, sy + TILE - 1, sx + TILE - 1, sy + TILE - 1, C_DARK_GRAY);
        lcd_px(sx + 1, sy + 1, C_DARK_GRAY);  lcd_px(sx + 14, sy + 1, C_DARK_GRAY);
        lcd_px(sx + 1, sy + 14, C_DARK_GRAY); lcd_px(sx + 14, sy + 14, C_DARK_GRAY);
        break;

    case T_MUSH:
        /* classic orange-brown block hiding a mushroom */
        lcd_rect(sx, sy, sx + TILE - 1, sy + TILE - 1, C_BROWN);
        lcd_rect(sx, sy, sx + TILE - 1, sy, C_BRICK_HI);
        lcd_rect(sx, sy + TILE - 1, sx + TILE - 1, sy + TILE - 1, C_DARK_GRAY);
        lcd_px(sx + 1, sy + 1, C_DARK_GRAY);  lcd_px(sx + 14, sy + 1, C_DARK_GRAY);
        lcd_px(sx + 1, sy + 14, C_DARK_GRAY); lcd_px(sx + 14, sy + 14, C_DARK_GRAY);
        lcd_sprite(mushroom_sprite[0], 12, 12, sx + 2, sy + 2, SPR_TRANSPARENT);
        break;

    case T_STAR:
        /* yellow ? block hiding a star */
        lcd_rect(sx, sy, sx + TILE - 1, sy + TILE - 1, C_YELLOW);
        lcd_rect(sx, sy, sx + TILE - 1, sy, C_BRICK_HI);
        lcd_rect(sx, sy + TILE - 1, sx + TILE - 1, sy + TILE - 1, C_DARK_GRAY);
        lcd_px(sx + 1, sy + 1, C_BROWN);   lcd_px(sx + 14, sy + 1, C_BROWN);
        lcd_px(sx + 1, sy + 14, C_BROWN);  lcd_px(sx + 14, sy + 14, C_BROWN);
        lcd_sprite(star_sprite[0], 12, 12, sx + 2, sy + 2, SPR_TRANSPARENT);
        break;

    case T_PIPE: {
        bool lip = (ty == 0) || (grid[ty - 1][tx] != T_PIPE);
        if (lip) {
            /* rim, overhanging both sides */
            lcd_rect(sx - 2, sy + 2, sx + TILE + 1, sy + 6, C_PIPE);
            lcd_rect(sx - 2, sy + 6, sx + TILE + 1, sy + 6, C_PIPE_DK);
        } else {
            /* stem with highlight and shadow stripes */
            lcd_rect(sx + 3, sy, sx + TILE - 4, sy + TILE - 1, C_PIPE);
            lcd_rect(sx + 3, sy, sx + 5, sy + TILE - 1, C_PIPE_DK);
            lcd_rect(sx + 10, sy, sx + 11, sy + TILE - 1, C_GREEN);
        }
        break;
    }

    case T_FLAG:
        /* pole from the flag cell down to the first solid tile */
        for (int tty = sy / TILE; tty < LEVEL_ROWS; tty++) {
            lcd_rect(sx + 2, tty * TILE, sx + 3, tty * TILE + TILE - 1, C_WHITE);
            if (solid_tile(grid[tty][tx])) break;
        }
        {
            int wf = (int)((frame >> 3) & 1);   /* waving flag */
            lcd_sprite(flag_sprite[wf][0], FLAG_W, FLAG_H,
                       sx + 4, sy + 2, SPR_TRANSPARENT);
        }
        /* brick castle to the right of the flag */
        {
            int gy = -1;
            for (int tty = sy / TILE; tty < LEVEL_ROWS; tty++)
                if (solid_tile(grid[tty][tx])) { gy = tty * TILE; break; }
            if (gy > 0) {
                int cx = sx + 12, cw = 44, ch = 40;
                lcd_rect(cx, gy - ch, cx + cw - 1, gy - 1, C_CASTLE);
                for (int yy = gy - ch; yy < gy; yy += 8)     /* brick rows */
                    lcd_rect(cx, yy, cx + cw - 1, yy, C_CASTLE_DK);
                for (int bx = cx; bx < cx + cw; bx += 11)    /* battlements */
                    lcd_rect(bx, gy - ch - 6, bx + 6, gy - ch - 1, C_CASTLE);
                lcd_rect(cx + cw / 2 - 6, gy - 18, cx + cw / 2 + 5, gy - 1,
                         C_CASTLE_DK);                         /* door */
                lcd_rect(cx + 6, gy - ch + 8, cx + 10, gy - ch + 12, C_CASTLE_DK);
                lcd_rect(cx + cw - 11, gy - ch + 8, cx + cw - 7, gy - ch + 12,
                         C_CASTLE_DK);                         /* windows */
            }
        }
        break;

    default:
        break;
    }
}

/* Mario faces right in the art; mirror manually when walking left.
 * Frame selection: jump pose in the air, walk cycle while moving,
 * standing otherwise. Big Mario (mushroom) draws from the taller
 * sprite set; hit invincibility blinks him on and off; the star
 * power-up cycles the shirt through a rainbow flash. */
static void draw_mario(int x, int y, bool facing_right)
{
    static const uint8_t walk_seq[4] = { 0, 1, 2, 1 };
    static const uint8_t flash_colors[4] = { P_R, P_Y, P_G, P_B };
    int f;
    if (!player.on_ground)
        f = 3;                           /* jump pose */
    else if (player.vx != 0)
        f = walk_seq[(frame >> 2) & 3];  /* walk cycle */
    else
        f = 0;                           /* standing */

    /* invincibility frames: blink (visible every other 4-frame phase) */
    if (iframes > 0 && ((frame >> 2) & 1) == 0)
        return;

    int h = player.big ? MARIO_BIG_H : MARIO_H;
    int w = player.big ? MARIO_BIG_W : MARIO_W;

    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            int src = facing_right ? col : (w - 1 - col);
            uint8_t c = player.big ? mario_big_sprite[f][row][src]
                                   : mario_sprite[f][row][src];
            if (c == SPR_TRANSPARENT)
                continue;
            /* star power: the shirt flashes through the palette */
            if (star_timer > 0 && c == P_R)
                c = flash_colors[(frame >> 2) & 3];
            lcd_px(x + col, y + row, c);
        }
    }
}

/* Triangular-wave mountain silhouette, one column at a time. */
static void draw_mountains(uint8_t color, int scroll, int amp, int base)
{
    for (int x = 0; x < LCD_W; x++) {
        int t = ((x + scroll) * 3) & 63;
        int h = (t < 32) ? t : (64 - t);
        lcd_rect(x, base - h - amp, x, 199, color);
    }
}

/* Puffy pixel cloud with a shaded base. s = size (1 or 2). */
static void draw_cloud(int x, int y, uint8_t c, uint8_t shade, int s)
{
    lcd_rect(x + 3 * s, y,          x + 8 * s,  y + 2 * s, c);
    lcd_rect(x + 1 * s, y + 2 * s,  x + 10 * s, y + 4 * s, c);
    lcd_rect(x,         y + 3 * s,  x + 11 * s, y + 6 * s, c);
    lcd_rect(x + 2 * s, y + 6 * s,  x + 9 * s,  y + 7 * s, shade);
}

static void render_world(void)
{
    int cam = cam_x + shake_x;   /* shake_x: +-2 px during the death screen */
    bool night = (level_idx == 4);   /* level 5 is the night level */

    /* full background repaint EVERY frame: the world scrolls, and a
     * static background would leave trails behind the moving tiles */
    {
        int top = night ? C_NIGHT_TOP : C_SKY_TOP;
        int hor = night ? C_NIGHT_HORIZ : C_SKY_HORIZON;
        for (int y = 0; y < 200; y++)
            lcd_rect(0, y, LCD_W - 1, y,
                     (uint8_t)(top + (199 - y) * (hor - top) / 199));
        for (int y = 200; y < LCD_H; y++)
            lcd_rect(0, y, LCD_W - 1, y, hor);
    }

    /* night level: a field of static stars */
    if (night) {
        for (int i = 0; i < 24; i++)
            lcd_px((i * 97) % 240, (i * 53) % 150 + 8, C_WHITE);
    }

    /* two parallax layers of NES-style green hills + drifting clouds */
    draw_mountains(night ? C_NIGHT_HILL_FAR : C_MOUNT_FAR,
                   cam / 6, 45, 150);
    draw_mountains(night ? C_NIGHT_HILL_NEAR : C_MOUNT_NEAR,
                   cam / 3, 30, 175);

    /* bushes along the horizon */
    for (int i = 0; i < 4; i++) {
        int bx = ((i * 80 - cam / 2) % 320 + 320) % 320 - 30;
        lcd_rect(bx, 186, bx + 24, 198, night ? C_NIGHT_HILL_NEAR : C_MOUNT_NEAR);
        lcd_rect(bx + 4, 182, bx + 16, 186, night ? C_NIGHT_HILL_NEAR : C_MOUNT_NEAR);
        lcd_rect(bx + 8, 179, bx + 13, 182, night ? C_NIGHT_HILL_FAR : C_MOUNT_FAR);
    }

    /* puffy clouds (day: white with a shaded base, night: gray) */
    uint8_t cc = night ? C_GRAY : C_WHITE;
    uint8_t cs = night ? C_DARK_GRAY : C_MAGENTA;
    int p1 = (cam / 6) % 280 - 20;
    int p2 = (cam / 4) % 280 - 20;
    draw_cloud(p1, 22, cc, cs, 1);
    draw_cloud(p2 + 90, 48, cc, cs, 2);
    draw_cloud(p1 + 160, 70, cc, cs, 1);

    int cam_tile = cam / TILE;
    int off = cam % TILE;

    for (int ty = 0; ty < LEVEL_ROWS; ty++) {
        for (int vx = 0; vx <= LCD_W / TILE; vx++) {
            int tx = cam_tile + vx;
            if (tx < 0 || tx >= LEVEL_COLS) continue;
            uint8_t t = grid[ty][tx];
            if (t == T_AIR) continue;
            draw_tile(vx * TILE - off, ty * TILE, tx, t);
        }
    }

    /* moving platforms */
    for (int i = 0; i < mover_count; i++) {
        mover_t *m = &movers[i];
        int mx = m->x - cam;
        lcd_rect(mx, m->y, mx + MOV_W - 1, m->y + MOV_H - 1, C_ORANGE);
        lcd_rect(mx, m->y, mx + MOV_W - 1, m->y, C_BRICK_HI);
        lcd_rect(mx, m->y + MOV_H - 1, mx + MOV_W - 1, m->y + MOV_H - 1, C_BLACK);
        lcd_px(mx + 3, m->y + MOV_H / 2, C_DARK_GRAY);
        lcd_px(mx + MOV_W - 4, m->y + MOV_H / 2, C_DARK_GRAY);
    }

    /* enemies */
    int ef = (int)((frame >> 4) & 1);   /* walk animation frame */
    for (int i = 0; i < enemy_count; i++) {
        enemy_t *e = &enemies[i];
        if (!e->alive) continue;
        if (e->flipped)
            lcd_sprite_flip_v(enemy_sprite[0][0], ENEMY_W, ENEMY_H,
                              e->x - cam, e->y, SPR_TRANSPARENT);
        else
            lcd_sprite(enemy_sprite[ef][0], ENEMY_W, ENEMY_H,
                       e->x - cam, e->y, SPR_TRANSPARENT);
    }

    /* power-up on the loose (mushroom walks, star bounces) */
    if (powup.active) {
        if (powup.star)
            lcd_sprite(star_sprite[0], 12, 12, powup.x - cam, powup.y,
                       SPR_TRANSPARENT);
        else
            lcd_sprite(mushroom_sprite[0], 12, 12, powup.x - cam, powup.y,
                       SPR_TRANSPARENT);
    }

    /* player */
    draw_mario(player.x - cam, player.y, player.facing_right);

    /* particles on top */
    render_particles();
}

static void draw_hud(void)
{
    lcd_rect(0, 0, LCD_W - 1, 11, C_BLACK);
    char buf[16];

    /* classic NES-style HUD: MARIO 000000 x3 ... LV */
    lcd_text(2, 2, "MARIO", C_WHITE, C_BLACK, 1);
    fmt_int_pad(buf, score, 6);
    lcd_text(38, 2, buf, C_WHITE, C_BLACK, 1);
    lcd_text(80, 2, "x", C_WHITE, C_BLACK, 1);
    fmt_int(buf, lives);
    lcd_text(88, 2, buf, C_WHITE, C_BLACK, 1);

    /* coin counter */
    lcd_text(112, 2, "C", C_YELLOW, C_BLACK, 1);
    fmt_int(buf, coins);
    lcd_text(120, 2, buf, C_WHITE, C_BLACK, 1);

    /* countdown timer */
    lcd_text(140, 2, "T", C_YELLOW, C_BLACK, 1);
    fmt_int(buf, time_left);
    lcd_text(146, 2, buf, C_WHITE, C_BLACK, 1);

    lcd_text(190, 2, "LV", C_WHITE, C_BLACK, 1);
    fmt_int(buf, level_idx + 1);
    lcd_text(208, 2, buf, C_WHITE, C_BLACK, 1);
}

/* ------------------------------ screens --------------------------- */

static void render_title(void)
{
    lcd_clear(C_SKY);
    lcd_rect(0, 200, LCD_W - 1, LCD_H - 1, C_DARK_GREEN);
    lcd_rect(0, 200, LCD_W - 1, 203, C_GREEN);

    lcd_text(60, 60, "MINI", C_RED, C_SKY, 3);
    lcd_text(48, 96, "MARIO", C_RED, C_SKY, 3);

    lcd_sprite(mario_sprite[0][0], MARIO_W, MARIO_H, 114, 140, SPR_TRANSPARENT);

    if (high_score > 0) {
        char buf[16];
        lcd_text(60, 172, "BEST", C_WHITE, C_SKY, 1);
        fmt_int(buf, high_score);
        lcd_text(90, 172, buf, C_YELLOW, C_SKY, 1);
    }

    lcd_text(24, 220, "LEFT/RIGHT: move", C_WHITE, C_DARK_GREEN, 1);
    lcd_text(24, 238, "B1 (blue): jump", C_WHITE, C_DARK_GREEN, 1);
    lcd_text(48, 268, "PRESS B1", C_YELLOW, C_DARK_GREEN, 2);
}

static void render_pause(void)
{
    /* overlay over the frozen frame */
    lcd_rect(0, 110, LCD_W - 1, 165, C_BLACK);
    lcd_text(72, 124, "PAUSED", C_WHITE, C_BLACK, 2);
    lcd_text(48, 148, "DOWN/B1: resume", C_GREEN, C_BLACK, 1);
}

static void render_level_clear(void)
{
    lcd_clear(C_BLACK);
    char buf[16];
    lcd_text(48, 90, "LEVEL", C_WHITE, C_BLACK, 3);
    fmt_int(buf, level_idx + 1);
    lcd_text(96, 126, buf, C_YELLOW, C_BLACK, 3);
    lcd_text(84, 170, "CLEAR!", C_GREEN, C_BLACK, 2);
    if (bonus > 0) {
        lcd_text(84, 202, "TIME", C_WHITE, C_BLACK, 1);
        lcd_text(114, 202, "BONUS", C_GREEN, C_BLACK, 1);
        fmt_int(buf, bonus);
        lcd_text(150, 202, buf, C_YELLOW, C_BLACK, 1);
    }
    fmt_int(buf, score);
    lcd_text(60, 240, "SCORE", C_WHITE, C_BLACK, 1);
    lcd_text(96, 258, buf, C_YELLOW, C_BLACK, 2);
}

static void render_game_over(void)
{
    lcd_clear(C_BLACK);
    lcd_text(72, 120, "GAME", C_RED, C_BLACK, 3);
    lcd_text(60, 156, "OVER", C_RED, C_BLACK, 3);
    char buf[16];
    fmt_int(buf, score);
    lcd_text(96, 210, "SCORE", C_WHITE, C_BLACK, 1);
    lcd_text(108, 228, buf, C_YELLOW, C_BLACK, 2);
    lcd_text(40, 280, "B1 = RETRY", C_GREEN, C_BLACK, 1);
}

static void render_win(void)
{
    lcd_clear(C_BLACK);
    lcd_text(48, 100, "YOU WIN!", C_YELLOW, C_BLACK, 3);
    char buf[16];
    fmt_int(buf, score);
    lcd_text(60, 180, "SCORE", C_WHITE, C_BLACK, 1);
    lcd_text(96, 198, buf, C_YELLOW, C_BLACK, 2);
    lcd_text(40, 260, "B1 = MENU", C_GREEN, C_BLACK, 1);
}

/* ------------------------------ main loop ------------------------- */

/*
 * High score persistence: the LAST 2 KB flash page (page 511 of the
 * 1 MB L476RG flash) stores a magic word + the score. Saved only at
 * game over / win, so the brief flash stall is never felt mid-game.
 */
#define HS_PAGE_BASE 0x080FF800UL
#define HS_MAGIC     0x4D415249UL   /* "MARI" */

static void flash_wait(void)
{
    volatile uint32_t *sr = (volatile uint32_t *)(0x40022000UL + 0x10);
    while (*sr & (1u << 16)) {}     /* BSY */
}

static void flash_unlock(void)
{
    volatile uint32_t *keyr = (volatile uint32_t *)(0x40022000UL + 0x08);
    *keyr = 0x45670123UL;
    *keyr = 0xCDEF89ABUL;
}

static void highscore_save(void)
{
    volatile uint32_t *cr = (volatile uint32_t *)(0x40022000UL + 0x14);

    flash_unlock();
    *cr |= (1u << 1);                       /* PER */
    *cr = (*cr & ~(0xFFu << 3)) | (511u << 3);   /* page 511 */
    *cr |= (1u << 16);                      /* STRT */
    flash_wait();
    *cr &= ~(1u << 1);
    *cr |= (1u << 0);                       /* PG */
    *(volatile uint32_t *)HS_PAGE_BASE = HS_MAGIC;
    flash_wait();
    *(volatile uint32_t *)(HS_PAGE_BASE + 4) = (uint32_t)high_score;
    flash_wait();
    *cr |= (1u << 31);                      /* LOCK */
}

static void highscore_load(void)
{
    if (*(volatile uint32_t *)HS_PAGE_BASE == HS_MAGIC)
        high_score = (int)*(volatile uint32_t *)(HS_PAGE_BASE + 4);
    else
        high_score = 0;
}

void game_run(void)
{
    score = 0;
    coins = 0;
    lives = 3;
    level_idx = 0;
    highscore_load();      /* best score survives power cycles */
    state = S_TITLE;

    for (;;) {
        frame++;

        /* screen shake decay */
        if (shake_timer > 0) {
            shake_x = (int)((frame * 7) & 3) - 2;
            shake_timer--;
        } else {
            shake_x = 0;
        }
        update_particles();
        /* the popping coin flies and falls */
        if (pop.life > 0) {
            pop.y += pop.vy;
            pop.vy += 1;
            pop.life--;
        }

        dir_t press = input_read();

        switch (state) {
        case S_TITLE:
            render_title();
            lcd_flush();
            if (press == DIR_CENTER) start_level(0);
            break;

        case S_INTRO:
            /* "WORLD N" card over the frozen world, then play */
            render_world();
            draw_hud();
            lcd_rect(36, 130, 204, 172, C_BLACK);
            {
                char buf[8];
                lcd_text(90, 138, "WORLD", C_WHITE, C_BLACK, 1);
                fmt_int(buf, level_idx + 1);
                lcd_text(110, 152, buf, C_YELLOW, C_BLACK, 2);
            }
            lcd_flush();
            if (--intro_timer <= 0) {
                state = S_PLAYING;
            }
            break;

        case S_PLAYING:
            if (press == DIR_DOWN) {
                state = S_PAUSED;      /* DOWN toggles pause */
                break;
            }
            /* power-up timers tick down */
            if (iframes > 0) iframes--;
            if (star_timer > 0) star_timer--;
            /* level countdown: one second per ~16 frames */
            if (++time_tick >= 16) {
                time_tick = 0;
                if (--time_left <= 0) {
                    time_left = 0;
                    player_die();      /* time is up: classic death */
                    break;
                }
            }
            update_movers();
            update_powerup();
            update_player();
            update_enemies();
            update_camera();
            render_world();
            draw_hud();
            lcd_flush();
            break;

        case S_PAUSED:
            render_pause();
            lcd_flush();
            if (press == DIR_DOWN || press == DIR_CENTER) {
                state = S_PLAYING;
            }
            break;

        case S_DEAD:
            /* classic death: Mario pops up and falls off the screen */
            player.vy += GRAVITY_FALL;
            if (player.vy > MAX_FALL) player.vy = MAX_FALL;
            player.y += player.vy;
            render_world();
            draw_hud();
            lcd_flush();
            if (--dead_timer <= 0) {
                lives--;
                if (lives <= 0) {
                    lives = 0;
                    if (score > high_score) {
                        high_score = score;
                        highscore_save();
                    }
                    state = S_GAME_OVER;
                } else {
                    start_level(level_idx);
                }
            }
            break;

        case S_CASTLE:
            /* flag grabbed: slide down the pole, walk to the door,
             * then slide into the castle. Enemies freeze. */
            update_movers();
            if (castle_phase == 0) {
                /* slide down the pole */
                player.y += 3;
                int feet = player.y + mario_h();
                if (box_hits(player.x + 1, feet - 1, MARIO_W - 2, 1)) {
                    player.y = (feet / TILE) * TILE - mario_h();
                    castle_phase = 1;          /* landed: walk */
                    auto_right = true;
                }
            } else if (castle_phase == 1) {
                update_player();
                if (player.x + MARIO_W / 2 >= flag_tx * TILE + 34) {
                    castle_phase = 2;          /* at the door: slide in */
                    entering_castle = true;
                    player.vx = 0;
                }
            } else {
                player.y += 4;                 /* slide down, off screen */
                if (player.y > LCD_H + 16) {
                    bonus = time_left * 5;     /* classic time bonus */
                    score += bonus;
                    flash_timer = 4;
                    clear_timer = 40;
                    auto_right = false;
                    state = S_LEVEL_CLEAR;
                }
            }
            update_camera();
            render_world();
            draw_hud();
            lcd_flush();
            break;

        case S_LEVEL_CLEAR:
            if (flash_timer > 0) {
                /* white flash first */
                lcd_clear(C_WHITE);
                flash_timer--;
            } else {
                render_level_clear();
            }
            lcd_flush();
            if (--clear_timer <= 0) {
                level_idx++;
                if (level_idx >= LEVEL_COUNT) {
                    if (score > high_score) {
                        high_score = score;
                        highscore_save();
                    }
                    state = S_WIN;
                } else {
                    start_level(level_idx);
                }
            }
            break;

        case S_GAME_OVER:
            render_game_over();
            lcd_flush();
            if (press == DIR_CENTER) {
                score = 0;
                coins = 0;
                lives = 3;
                level_idx = 0;
                start_level(0);
            }
            break;

        case S_WIN:
            render_win();
            lcd_flush();
            if (press == DIR_CENTER) {
                score = 0;
                coins = 0;
                lives = 3;
                level_idx = 0;
                state = S_TITLE;
            }
            break;
        }
    }
}
