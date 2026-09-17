/*
 * game.c — Mini-Mario.
 *
 * A small side-scrolling platformer:
 *   - 16 px tile grid, 240x320 view, horizontal camera
 *   - player physics: gravity, jumping, axis-separated collision
 *   - walker enemies (stomp them), coins, a goal flag, 3 levels
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

/* tile kinds */
enum { T_AIR = '.', T_GROUND = '#', T_BRICK = 'B',
       T_COIN = 'C', T_ENEMY = 'E', T_FLAG = 'F', T_PLAYER = 'P' };

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
    ".....................E..........................................",
    "..........CCC.......BBBB......CCC.......BBB.......CCC...........",
    "................................................................",
    "..........BBB.................BBB.................BBB...........",
    "..P...............................C.C.C.........................",
    "################################################################",
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
    "..................................E.............................",
    "........................BBB......BBB............................",
    "................................................................",
    ".................BB...C.......C.................C...............",
    "..P...............................................E.............",
    "###############....######################....###################",
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
    "..........................CCC.....BBB.....CCC.....BBB...........",
    "...........................E...............E....................",
    "......BB..................BBB.............BBB...................",
    "........................................C..............C........",
    ".P.E............................................................",
    "###########...#######...########################################",
    "###########...#######...########################################",
};

static const char *const levels[] = { &level1[0][0], &level2[0][0], &level3[0][0] };
#define LEVEL_COUNT 3

/* mutable copy of the current level (coins get removed, etc.) */
static uint8_t grid[LEVEL_ROWS][LEVEL_COLS];

/* ------------------------------ entities -------------------------- */

typedef struct {
    int  x, y;          /* top-left, pixels */
    int  vx, vy;        /* px per frame */
    bool on_ground;
    bool facing_right;
} player_t;

typedef struct {
    int  x, y;
    int  vx;
    bool alive;
} enemy_t;

static player_t player;
static enemy_t  enemies[4];
static int      enemy_count;

/* ------------------------------ game state ------------------------ */

typedef enum { S_TITLE, S_PLAYING, S_GAME_OVER, S_WIN } state_t;

static state_t state;
static int     level_idx;
static int     score;
static int     lives;
static int     cam_x;
static uint32_t frame;       /* frame counter (coin animation) */

/* ------------------------------ prototypes ------------------------ */

static bool solid_at(int tx, int ty);
static bool box_hits(int x, int y, int w, int h);
static bool overlap(int ax, int ay, int aw, int ah,
                    int bx, int by, int bw, int bh);
static void player_die(void);
static void start_level(int n);
static void draw_mario(int x, int y, bool facing_right);

/* ------------------------------ level loading --------------------- */

static bool solid_tile(uint8_t t)
{
    return t == T_GROUND || t == T_BRICK;
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

static void start_level(int n)
{
    const char *src = levels[n];
    enemy_count = 0;

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
                    enemies[enemy_count].alive = true;
                    enemy_count++;
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
    cam_x = 0;
    state = S_PLAYING;
}

/* ------------------------------ physics --------------------------- */

#define GRAVITY      1
#define MAX_FALL     12
#define JUMP_VEL     -10
#define MOVE_SPEED   2

static void update_player(void)
{
    bool left  = input_held(DIR_LEFT);
    bool right = input_held(DIR_RIGHT);

    player.vx = (right ? MOVE_SPEED : 0) - (left ? MOVE_SPEED : 0);
    if (player.vx > 0) player.facing_right = true;
    if (player.vx < 0) player.facing_right = false;

    /* jump: CENTER (currently the blue B1 button) */
    if (input_held(DIR_CENTER) && player.on_ground)
        player.vy = JUMP_VEL;

    /* gravity */
    player.vy += GRAVITY;
    if (player.vy > MAX_FALL) player.vy = MAX_FALL;

    /* --- vertical move with collision --- */
    if (player.vy >= 0) {
        /* falling: check the feet line */
        int ny = player.y + player.vy;
        int feet = ny + MARIO_H;
        if (box_hits(player.x + 1, feet - 1, MARIO_W - 2, 1)) {
            player.y = (feet / TILE) * TILE - MARIO_H;   /* snap on top */
            player.vy = 0;
            player.on_ground = true;
        } else {
            player.y = ny;
            player.on_ground = false;
        }
    } else {
        /* rising: check the head line */
        int ny = player.y + player.vy;
        if (box_hits(player.x + 1, ny, MARIO_W - 2, 1)) {
            player.y = (ny / TILE + 1) * TILE;           /* snap below */
            player.vy = 0;
        } else {
            player.y = ny;
        }
    }

    /* --- horizontal move with collision --- */
    if (player.vx != 0) {
        int nx = player.x + player.vx;
        if (box_hits(nx, player.y + 1, MARIO_W, MARIO_H - 2)) {
            /* push against the wall */
            if (player.vx > 0) player.x = (nx / TILE) * TILE - MARIO_W;
            else               player.x = (nx / TILE + 1) * TILE;
            player.vx = 0;
        } else {
            player.x = nx;
        }
    }

    /* --- collect coins the player overlaps --- */
    int x0 = player.x / TILE, x1 = (player.x + MARIO_W - 1) / TILE;
    int y0 = player.y / TILE, y1 = (player.y + MARIO_H - 1) / TILE;
    for (int ty = y0; ty <= y1; ty++)
        for (int tx = x0; tx <= x1; tx++) {
            if (tx < 0 || tx >= LEVEL_COLS || ty < 0 || ty >= LEVEL_ROWS) continue;
            if (grid[ty][tx] == T_COIN) {
                grid[ty][tx] = T_AIR;
                score += 10;
            }
        }

    /* --- reached the goal flag? --- */
    int ptx = (player.x + MARIO_W / 2) / TILE;
    for (int ty = 0; ty < LEVEL_ROWS; ty++) {
        if (grid[ty][ptx] == T_FLAG) {
            level_idx++;
            if (level_idx >= LEVEL_COUNT) state = S_WIN;
            else start_level(level_idx);
            return;
        }
    }

    /* --- fell into a pit --- */
    if (player.y > LCD_H + 16)
        player_die();
}

static void update_enemies(void)
{
    for (int i = 0; i < enemy_count; i++) {
        enemy_t *e = &enemies[i];
        if (!e->alive) continue;

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
        if (overlap(player.x, player.y, MARIO_W, MARIO_H,
                    e->x, e->y, ENEMY_W, ENEMY_H)) {
            if (player.vy > 0 &&
                (player.y + MARIO_H - e->y) < 10) {
                /* stomp! */
                e->alive = false;
                player.vy = -8;
                score += 50;
            } else {
                player_die();
                return;
            }
        }
    }
}

static void player_die(void)
{
    lives--;
    if (lives <= 0) {
        lives = 0;
        state = S_GAME_OVER;
    } else {
        start_level(level_idx);
    }
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

static void draw_tile(int sx, int sy, int tx, uint8_t t)
{
    switch (t) {
    case T_GROUND:
        lcd_rect(sx, sy, sx + TILE - 1, sy + TILE - 1, C_BROWN);
        lcd_rect(sx, sy, sx + TILE - 1, sy + 2, C_DARK_GREEN);   /* grass */
        break;

    case T_BRICK:
        lcd_rect(sx, sy, sx + TILE - 1, sy + TILE - 1, C_ORANGE);
        lcd_rect(sx, sy + 5, sx + TILE - 1, sy + 5, C_DARK_GRAY);  /* mortar */
        lcd_rect(sx + 5, sy, sx + 5, sy + 4, C_DARK_GRAY);
        lcd_rect(sx + 11, sy + 6, sx + 11, sy + TILE - 1, C_DARK_GRAY);
        break;

    case T_COIN: {
        int f = (int)((frame >> 3) & 1);
        lcd_sprite(coin_sprite[f][0], COIN_W, COIN_H,
                   sx + 4, sy + 4, SPR_TRANSPARENT);
        break;
    }

    case T_FLAG:
        /* pole from the flag cell down to the first solid tile */
        for (int ty = sy / TILE; ty < LEVEL_ROWS; ty++) {
            lcd_rect(sx + 2, ty * TILE, sx + 3, ty * TILE + TILE - 1, C_WHITE);
            if (solid_tile(grid[ty][tx])) break;
        }
        lcd_sprite(flag_sprite[0], FLAG_W, FLAG_H,
                   sx + 4, sy + 2, SPR_TRANSPARENT);
        break;

    default:
        break;
    }
}

/* Mario faces right in the art; mirror manually when walking left. */
static void draw_mario(int x, int y, bool facing_right)
{
    for (int row = 0; row < MARIO_H; row++) {
        for (int col = 0; col < MARIO_W; col++) {
            int src = facing_right ? col : (MARIO_W - 1 - col);
            uint8_t c = mario_sprite[row][src];
            if (c != SPR_TRANSPARENT)
                lcd_px(x + col, y + row, c);
        }
    }
}

static void render_world(void)
{
    lcd_clear(C_SKY);

    /* decorative parallax clouds */
    int p1 = (cam_x / 6) % 260 - 20;
    int p2 = (cam_x / 4) % 260 - 20;
    lcd_rect(p1, 22, p1 + 30, 32, C_WHITE);
    lcd_rect(p2 + 90, 48, p2 + 126, 60, C_WHITE);

    int cam_tile = cam_x / TILE;
    int off = cam_x % TILE;

    for (int ty = 0; ty < LEVEL_ROWS; ty++) {
        for (int vx = 0; vx <= LCD_W / TILE; vx++) {
            int tx = cam_tile + vx;
            if (tx < 0 || tx >= LEVEL_COLS) continue;
            uint8_t t = grid[ty][tx];
            if (t == T_AIR) continue;
            draw_tile(vx * TILE - off, ty * TILE, tx, t);
        }
    }

    /* enemies */
    for (int i = 0; i < enemy_count; i++) {
        enemy_t *e = &enemies[i];
        if (!e->alive) continue;
        lcd_sprite(enemy_sprite[0], ENEMY_W, ENEMY_H,
                   e->x - cam_x, e->y, SPR_TRANSPARENT);
    }

    /* player */
    draw_mario(player.x - cam_x, player.y, player.facing_right);
}

static void draw_hud(void)
{
    lcd_rect(0, 0, LCD_W - 1, 11, C_BLACK);
    char buf[16];

    lcd_text(2, 2, "SCORE", C_WHITE, C_BLACK, 1);
    fmt_int(buf, score);
    lcd_text(36, 2, buf, C_YELLOW, C_BLACK, 1);

    lcd_text(120, 2, "LIVES", C_WHITE, C_BLACK, 1);
    fmt_int(buf, lives);
    lcd_text(152, 2, buf, C_RED, C_BLACK, 1);

    lcd_text(190, 2, "LV", C_WHITE, C_BLACK, 1);
    fmt_int(buf, level_idx + 1);
    lcd_text(208, 2, buf, C_GREEN, C_BLACK, 1);
}

/* ------------------------------ screens --------------------------- */

static void render_title(void)
{
    lcd_clear(C_SKY);
    lcd_rect(0, 200, LCD_W - 1, LCD_H - 1, C_DARK_GREEN);
    lcd_rect(0, 200, LCD_W - 1, 203, C_GREEN);

    lcd_text(60, 60, "MINI", C_RED, C_SKY, 3);
    lcd_text(48, 96, "MARIO", C_RED, C_SKY, 3);

    lcd_sprite(mario_sprite[0], MARIO_W, MARIO_H, 114, 140, SPR_TRANSPARENT);

    lcd_text(24, 220, "LEFT/RIGHT: move", C_WHITE, C_DARK_GREEN, 1);
    lcd_text(24, 238, "CENTER: jump", C_WHITE, C_DARK_GREEN, 1);
    lcd_text(24, 262, "PRESS CENTER TO START", C_YELLOW, C_DARK_GREEN, 2);
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
    lcd_text(40, 280, "CENTER = RETRY", C_GREEN, C_BLACK, 1);
}

static void render_win(void)
{
    lcd_clear(C_BLACK);
    lcd_text(48, 100, "YOU WIN!", C_YELLOW, C_BLACK, 3);
    char buf[16];
    fmt_int(buf, score);
    lcd_text(60, 180, "SCORE", C_WHITE, C_BLACK, 1);
    lcd_text(96, 198, buf, C_YELLOW, C_BLACK, 2);
    lcd_text(40, 260, "CENTER = MENU", C_GREEN, C_BLACK, 1);
}

/* ------------------------------ main loop ------------------------- */

void game_run(void)
{
    score = 0;
    lives = 3;
    level_idx = 0;
    state = S_TITLE;

    for (;;) {
        frame++;
        dir_t press = input_read();

        switch (state) {
        case S_TITLE:
            render_title();
            lcd_flush();
            if (press == DIR_CENTER) start_level(0);
            break;

        case S_PLAYING:
            update_player();
            update_enemies();
            update_camera();
            render_world();
            draw_hud();
            lcd_flush();
            break;

        case S_GAME_OVER:
            render_game_over();
            lcd_flush();
            if (press == DIR_CENTER) {
                score = 0;
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
                lives = 3;
                level_idx = 0;
                state = S_TITLE;
            }
            break;
        }
    }
}
