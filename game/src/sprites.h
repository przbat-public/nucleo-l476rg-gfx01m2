/*
 * sprites.h — pixel art as palette-index arrays, in the style of the
 * classic NES Mario look (original art inspired by, not copied from,
 * Nintendo's sprites).
 *
 * Sprites use lcd.h palette indices. Index SPR_TRANSPARENT (255) is
 * skipped when drawing. Mario faces RIGHT; the renderer mirrors him
 * when he walks left.
 */
#pragma once
#include <stdint.h>

#define SPR_TRANSPARENT 0xFF

/* palette shortcuts for readable art */
#define P_  SPR_TRANSPARENT
#define P_K 0   /* black outline    */
#define P_R 2   /* mario red        */
#define P_B 4   /* overalls blue    */
#define P_S 13  /* skin             */
#define P_N 8   /* gray             */
#define P_W 1   /* white            */
#define P_BR 11 /* brown            */
#define P_G 12  /* gold             */
#define P_DG 15 /* dark green       */
#define P_O 14 /* brick             */
#define P_Y 5   /* yellow           */
#define P_H 9   /* hair/shoes brown */

/* Mario, 12 x 16, three walk frames + a jump pose.
 * Cap with a brim, mustache, blue overalls with yellow buttons,
 * brown shoes. */
#define MARIO_W 12
#define MARIO_H 16
#define MARIO_FRAMES 4    /* 0..2 = walk, 3 = jump */

static const uint8_t mario_sprite[MARIO_FRAMES][MARIO_H][MARIO_W] = {
    { /* 0: standing */
        { P_, P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_, P_ },
        { P_, P_, P_R, P_R, P_W, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_, P_H, P_H, P_H, P_H, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_S, P_K, P_S, P_S, P_K, P_S, P_S, P_, P_, P_ },
        { P_, P_, P_S, P_S, P_S, P_H, P_H, P_H, P_S, P_S, P_, P_ },
        { P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_ },
        { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_Y, P_Y, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_K, P_K, P_, P_, P_K, P_K, P_, P_, P_, P_ },
        { P_, P_K, P_K, P_K, P_, P_, P_K, P_K, P_K, P_, P_, P_ },
    },
    { /* 1: walk — left leg forward */
        { P_, P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_, P_ },
        { P_, P_, P_R, P_R, P_W, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_, P_H, P_H, P_H, P_H, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_S, P_K, P_S, P_S, P_K, P_S, P_S, P_, P_, P_ },
        { P_, P_, P_S, P_S, P_S, P_H, P_H, P_H, P_S, P_S, P_, P_ },
        { P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_ },
        { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_Y, P_Y, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_K, P_K, P_K, P_, P_, P_, P_K, P_K, P_, P_, P_ },
        { P_, P_K, P_K, P_K, P_, P_, P_, P_K, P_K, P_, P_, P_ },
    },
    { /* 2: walk — legs together */
        { P_, P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_, P_ },
        { P_, P_, P_R, P_R, P_W, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_, P_H, P_H, P_H, P_H, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_S, P_K, P_S, P_S, P_K, P_S, P_S, P_, P_, P_ },
        { P_, P_, P_S, P_S, P_S, P_H, P_H, P_H, P_S, P_S, P_, P_ },
        { P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_ },
        { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_Y, P_Y, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_, P_, P_K, P_K, P_K, P_K, P_, P_, P_, P_ },
        { P_, P_, P_, P_K, P_K, P_K, P_K, P_K, P_K, P_, P_, P_ },
    },
    { /* 3: jump — arms out, legs tucked */
        { P_, P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_, P_ },
        { P_, P_, P_R, P_R, P_W, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_, P_H, P_H, P_H, P_H, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_S, P_K, P_S, P_S, P_K, P_S, P_S, P_, P_, P_ },
        { P_, P_, P_S, P_S, P_S, P_H, P_H, P_H, P_S, P_S, P_, P_ },
        { P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_ },
        { P_, P_R, P_R, P_, P_R, P_R, P_R, P_R, P_, P_R, P_R, P_ },
        { P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_Y, P_Y, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_K, P_K, P_K, P_K, P_K, P_K, P_, P_, P_, P_ },
        { P_, P_, P_, P_K, P_K, P_K, P_K, P_, P_, P_, P_, P_ },
    },
};

/* Goomba-like walker, 12 x 12 (two walk frames).
 * Brown mushroom body, angry eyes, black feet. */
#define ENEMY_W 12
#define ENEMY_H 12
static const uint8_t enemy_sprite[2][ENEMY_H][ENEMY_W] = {
    { /* frame 0 */
        { P_, P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_, P_ },
        { P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_ },
        { P_BR, P_K, P_K, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_K, P_K, P_BR },
        { P_BR, P_W, P_W, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_W, P_W, P_BR },
        { P_BR, P_W, P_K, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_K, P_W, P_BR },
        { P_BR, P_W, P_W, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_W, P_W, P_BR },
        { P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_ },
        { P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_ },
        { P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_ },
        { P_K, P_K, P_K, P_, P_, P_, P_, P_, P_, P_K, P_K, P_K },
        { P_K, P_K, P_K, P_K, P_, P_, P_, P_, P_K, P_K, P_K, P_K },
        { P_K, P_K, P_K, P_K, P_, P_, P_, P_, P_K, P_K, P_K, P_K },
    },
    { /* frame 1 (feet swapped) */
        { P_, P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_, P_ },
        { P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_ },
        { P_BR, P_K, P_K, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_K, P_K, P_BR },
        { P_BR, P_W, P_W, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_W, P_W, P_BR },
        { P_BR, P_W, P_K, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_K, P_W, P_BR },
        { P_BR, P_W, P_W, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_W, P_W, P_BR },
        { P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_ },
        { P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_ },
        { P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_ },
        { P_K, P_K, P_K, P_K, P_, P_, P_, P_, P_K, P_K, P_K, P_K },
        { P_K, P_K, P_K, P_, P_, P_, P_, P_, P_, P_K, P_K, P_K },
        { P_K, P_K, P_K, P_, P_, P_, P_, P_, P_, P_K, P_K, P_K },
    },
};

/* Coin, 8 x 8, four rotation frames */
#define COIN_W 8
#define COIN_H 8
static const uint8_t coin_sprite[4][COIN_H][COIN_W] = {
    { /* full face */
        { P_, P_, P_G, P_G, P_G, P_G, P_, P_ },
        { P_, P_G, P_W, P_G, P_G, P_W, P_G, P_ },
        { P_G, P_W, P_, P_, P_, P_, P_W, P_G },
        { P_G, P_W, P_, P_, P_, P_, P_W, P_G },
        { P_G, P_W, P_, P_, P_, P_, P_W, P_G },
        { P_G, P_W, P_, P_, P_, P_, P_W, P_G },
        { P_, P_G, P_W, P_G, P_G, P_W, P_G, P_ },
        { P_, P_, P_G, P_G, P_G, P_G, P_, P_ },
    },
    { /* three-quarter */
        { P_, P_, P_, P_G, P_G, P_, P_, P_ },
        { P_, P_, P_G, P_W, P_W, P_G, P_, P_ },
        { P_, P_G, P_W, P_, P_, P_W, P_G, P_ },
        { P_, P_G, P_W, P_, P_, P_W, P_G, P_ },
        { P_, P_G, P_W, P_, P_, P_W, P_G, P_ },
        { P_, P_, P_G, P_W, P_W, P_G, P_, P_ },
        { P_, P_, P_, P_G, P_G, P_, P_, P_ },
        { P_, P_, P_, P_, P_, P_, P_, P_ },
    },
    { /* edge */
        { P_, P_, P_, P_, P_, P_, P_, P_ },
        { P_, P_, P_, P_G, P_G, P_, P_, P_ },
        { P_, P_, P_, P_G, P_G, P_, P_, P_ },
        { P_, P_, P_, P_W, P_W, P_, P_, P_ },
        { P_, P_, P_, P_W, P_W, P_, P_, P_ },
        { P_, P_, P_, P_G, P_G, P_, P_, P_ },
        { P_, P_, P_, P_G, P_G, P_, P_, P_ },
        { P_, P_, P_, P_, P_, P_, P_, P_ },
    },
    { /* three-quarter (other side) */
        { P_, P_, P_G, P_G, P_, P_, P_, P_ },
        { P_, P_G, P_W, P_W, P_G, P_, P_, P_ },
        { P_G, P_W, P_, P_, P_W, P_G, P_, P_ },
        { P_G, P_W, P_, P_, P_W, P_G, P_, P_ },
        { P_G, P_W, P_, P_, P_W, P_G, P_, P_ },
        { P_, P_G, P_W, P_W, P_G, P_, P_, P_ },
        { P_, P_, P_G, P_G, P_, P_, P_, P_ },
        { P_, P_, P_, P_, P_, P_, P_, P_ },
    },
};

/* Goal flag, 8 x 14 (two wave frames) */
#define FLAG_W 8
#define FLAG_H 14
static const uint8_t flag_sprite[2][FLAG_H][FLAG_W] = {
    { /* wave 0 */
        { P_, P_, P_, P_, P_, P_R, P_R, P_R },
        { P_, P_, P_, P_, P_, P_R, P_W, P_W },
        { P_, P_, P_, P_, P_, P_R, P_R, P_R },
        { P_, P_, P_, P_, P_, P_R, P_W, P_W },
        { P_, P_, P_, P_, P_, P_R, P_R, P_R },
        { P_, P_, P_, P_, P_, P_R, P_W, P_W },
        { P_, P_, P_, P_, P_, P_R, P_R, P_R },
        { P_, P_, P_, P_, P_, P_R, P_W, P_W },
        { P_, P_, P_, P_, P_, P_R, P_R, P_R },
        { P_, P_, P_, P_, P_, P_R, P_W, P_W },
        { P_, P_, P_, P_, P_, P_R, P_R, P_R },
        { P_, P_, P_, P_, P_, P_W, P_W, P_W },
        { P_, P_, P_, P_, P_, P_W, P_W, P_W },
        { P_, P_, P_, P_, P_, P_W, P_W, P_W },
    },
    { /* wave 1 (flag shifted left) */
        { P_, P_, P_, P_, P_R, P_R, P_R, P_ },
        { P_, P_, P_, P_, P_R, P_W, P_W, P_ },
        { P_, P_, P_, P_, P_R, P_R, P_R, P_ },
        { P_, P_, P_, P_, P_R, P_W, P_W, P_ },
        { P_, P_, P_, P_, P_R, P_R, P_R, P_ },
        { P_, P_, P_, P_, P_R, P_W, P_W, P_ },
        { P_, P_, P_, P_, P_R, P_R, P_R, P_ },
        { P_, P_, P_, P_, P_R, P_W, P_W, P_ },
        { P_, P_, P_, P_, P_R, P_R, P_R, P_ },
        { P_, P_, P_, P_, P_R, P_W, P_W, P_ },
        { P_, P_, P_, P_, P_R, P_R, P_R, P_ },
        { P_, P_, P_, P_, P_W, P_W, P_W, P_ },
        { P_, P_, P_, P_, P_W, P_W, P_W, P_ },
        { P_, P_, P_, P_, P_W, P_W, P_W, P_ },
    },
};


/* Big Mario, 12 x 28 (two tiles tall), four frames: same palette,
 * same hat. Active while the mushroom power-up is held. */
#define MARIO_BIG_W 12
#define MARIO_BIG_H 28
#define MARIO_BIG_FRAMES 4

static const uint8_t mario_big_sprite[MARIO_BIG_FRAMES][MARIO_BIG_H][MARIO_BIG_W] = {
    { /* 0: standing */
        { P_, P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_, P_ },
        { P_, P_, P_R, P_R, P_W, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_H, P_H, P_H, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_ },
        { P_, P_H, P_H, P_H, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_ },
        { P_, P_H, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_ },
        { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_S, P_K, P_S, P_S, P_K, P_S, P_S, P_, P_, P_ },
        { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_S, P_S, P_S, P_H, P_H, P_H, P_S, P_S, P_S, P_ },
        { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_ },
        { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_R, P_R, P_R, P_B, P_B, P_B, P_B, P_R, P_R, P_R, P_ },
        { P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_Y, P_Y, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_K, P_K, P_, P_, P_K, P_K, P_, P_, P_, P_ },
        { P_, P_, P_K, P_K, P_, P_, P_K, P_K, P_, P_, P_, P_ },
        { P_, P_K, P_K, P_K, P_, P_, P_, P_K, P_K, P_K, P_, P_ },
        { P_, P_K, P_K, P_K, P_, P_, P_, P_K, P_K, P_K, P_, P_ },
    },
    { /* 1: walk */
        { P_, P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_, P_ },
        { P_, P_, P_R, P_R, P_W, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_H, P_H, P_H, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_ },
        { P_, P_H, P_H, P_H, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_ },
        { P_, P_H, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_ },
        { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_S, P_K, P_S, P_S, P_K, P_S, P_S, P_, P_, P_ },
        { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_S, P_S, P_S, P_H, P_H, P_H, P_S, P_S, P_S, P_ },
        { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_ },
        { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_R, P_R, P_R, P_B, P_B, P_B, P_B, P_R, P_R, P_R, P_ },
        { P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_Y, P_Y, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_, P_B, P_B, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_, P_B, P_B, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_, P_B, P_B, P_, P_, P_ },
        { P_, P_K, P_K, P_K, P_, P_, P_, P_, P_K, P_K, P_, P_ },
        { P_, P_K, P_K, P_K, P_, P_, P_, P_, P_K, P_K, P_, P_ },
        { P_K, P_K, P_K, P_K, P_, P_, P_, P_, P_K, P_K, P_K, P_ },
        { P_, P_, P_, P_, P_, P_, P_, P_, P_K, P_K, P_K, P_ },
    },
    { /* 2: walk 2 */
        { P_, P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_, P_ },
        { P_, P_, P_R, P_R, P_W, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_H, P_H, P_H, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_ },
        { P_, P_H, P_H, P_H, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_ },
        { P_, P_H, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_ },
        { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_S, P_K, P_S, P_S, P_K, P_S, P_S, P_, P_, P_ },
        { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_S, P_S, P_S, P_H, P_H, P_H, P_S, P_S, P_S, P_ },
        { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_ },
        { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_R, P_R, P_R, P_B, P_B, P_B, P_B, P_R, P_R, P_R, P_ },
        { P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_Y, P_Y, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_, P_, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_, P_K, P_K, P_K, P_K, P_, P_, P_, P_, P_ },
        { P_, P_, P_, P_K, P_K, P_K, P_K, P_, P_, P_, P_, P_ },
        { P_, P_, P_K, P_K, P_K, P_K, P_K, P_K, P_, P_, P_, P_ },
        { P_, P_, P_K, P_K, P_K, P_K, P_K, P_K, P_, P_, P_, P_ },
    },
    { /* 3: jump */
        { P_, P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_, P_ },
        { P_, P_, P_R, P_R, P_W, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
        { P_, P_H, P_H, P_H, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_ },
        { P_, P_H, P_H, P_H, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_ },
        { P_, P_H, P_H, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_ },
        { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_S, P_K, P_S, P_S, P_K, P_S, P_S, P_, P_, P_ },
        { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_, P_S, P_S, P_S, P_H, P_H, P_H, P_S, P_S, P_S, P_ },
        { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
        { P_, P_R, P_R, P_, P_R, P_R, P_R, P_R, P_, P_R, P_R, P_ },
        { P_R, P_R, P_, P_, P_R, P_R, P_R, P_R, P_, P_, P_R, P_R },
        { P_R, P_R, P_, P_, P_R, P_R, P_R, P_R, P_, P_, P_R, P_R },
        { P_, P_R, P_R, P_R, P_B, P_B, P_B, P_B, P_R, P_R, P_R, P_ },
        { P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_Y, P_Y, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_, P_, P_ },
        { P_, P_, P_K, P_K, P_K, P_K, P_K, P_K, P_, P_, P_, P_ },
        { P_, P_, P_, P_K, P_K, P_K, P_K, P_, P_, P_, P_, P_ },
        { P_, P_, P_, P_K, P_K, P_K, P_K, P_, P_, P_, P_, P_ },
        { P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_ },
        { P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_ },
    },
};

/* Mushroom power-up, 12 x 12: red cap, white spots, tan stem. */
static const uint8_t mushroom_sprite[12][12] = {
    { P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_ },
    { P_, P_R, P_R, P_W, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_ },
    { P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R },
    { P_R, P_R, P_W, P_R, P_R, P_R, P_R, P_W, P_R, P_R, P_R, P_R },
    { P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_R },
    { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
    { P_, P_, P_S, P_K, P_S, P_S, P_S, P_S, P_K, P_S, P_, P_ },
    { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
    { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
    { P_, P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_, P_ },
    { P_, P_, P_, P_, P_S, P_S, P_S, P_S, P_, P_, P_, P_ },
    { P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_ },
};

/* Star power-up, 12 x 12: classic yellow star with eyes. */
static const uint8_t star_sprite[12][12] = {
    { P_, P_, P_, P_, P_, P_Y, P_Y, P_, P_, P_, P_, P_ },
    { P_, P_, P_, P_, P_Y, P_Y, P_Y, P_Y, P_, P_, P_, P_ },
    { P_, P_, P_, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_, P_, P_ },
    { P_, P_, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_, P_ },
    { P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y },
    { P_, P_Y, P_Y, P_Y, P_K, P_K, P_Y, P_Y, P_Y, P_Y, P_, P_ },
    { P_, P_, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_, P_ },
    { P_, P_, P_, P_Y, P_Y, P_Y, P_Y, P_Y, P_Y, P_, P_, P_ },
    { P_, P_, P_, P_Y, P_Y, P_, P_, P_Y, P_Y, P_, P_, P_ },
    { P_, P_, P_Y, P_Y, P_, P_, P_, P_, P_Y, P_Y, P_, P_ },
    { P_, P_Y, P_Y, P_, P_, P_, P_, P_, P_, P_Y, P_Y, P_ },
    { P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_ },
};

/* Fireball, 8 x 8, two swirling frames. */
#define FIRE_W 8
#define FIRE_H 8
static const uint8_t fire_sprite[2][FIRE_H][FIRE_W] = {
    { /* frame 0 */
        { P_, P_, P_O, P_O, P_, P_, P_, P_ },
        { P_, P_O, P_Y, P_Y, P_O, P_, P_, P_ },
        { P_, P_O, P_Y, P_Y, P_Y, P_O, P_, P_ },
        { P_, P_O, P_Y, P_Y, P_Y, P_Y, P_, P_ },
        { P_, P_, P_O, P_Y, P_Y, P_Y, P_O, P_ },
        { P_, P_, P_, P_O, P_Y, P_O, P_, P_ },
        { P_, P_, P_, P_, P_O, P_O, P_, P_ },
        { P_, P_, P_, P_, P_, P_, P_, P_ },
    },
    { /* frame 1 (swirl) */
        { P_, P_, P_, P_, P_O, P_O, P_, P_ },
        { P_, P_, P_, P_O, P_Y, P_Y, P_O, P_ },
        { P_, P_O, P_O, P_Y, P_Y, P_Y, P_, P_ },
        { P_, P_O, P_Y, P_Y, P_Y, P_O, P_, P_ },
        { P_, P_O, P_Y, P_Y, P_O, P_, P_, P_ },
        { P_, P_, P_O, P_Y, P_Y, P_O, P_, P_ },
        { P_, P_, P_, P_O, P_O, P_, P_, P_ },
        { P_, P_, P_, P_, P_, P_, P_, P_ },
    },
};

/* Green 1-UP mushroom, 12 x 12: same shape, green cap. */
static const uint8_t mushroom_1up_sprite[12][12] = {
    { P_, P_, P_G, P_G, P_G, P_G, P_G, P_G, P_G, P_G, P_, P_ },
    { P_, P_G, P_G, P_W, P_G, P_G, P_G, P_G, P_G, P_G, P_G, P_ },
    { P_G, P_G, P_G, P_G, P_G, P_G, P_G, P_G, P_G, P_G, P_G, P_G },
    { P_G, P_G, P_W, P_G, P_G, P_G, P_G, P_W, P_G, P_G, P_G, P_G },
    { P_G, P_G, P_G, P_G, P_G, P_G, P_G, P_G, P_G, P_G, P_G, P_G },
    { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
    { P_, P_, P_S, P_K, P_S, P_S, P_S, P_S, P_K, P_S, P_, P_ },
    { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
    { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_ },
    { P_, P_, P_, P_S, P_S, P_S, P_S, P_S, P_, P_, P_, P_ },
    { P_, P_, P_, P_, P_S, P_S, P_S, P_, P_, P_, P_, P_ },
    { P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_ },
};
