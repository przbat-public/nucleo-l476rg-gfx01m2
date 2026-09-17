/*
 * sprites.h — pixel art as palette-index arrays.
 *
 * Sprites use lcd.h palette indices (C_RED, C_BLUE, ...).
 * Index SPR_TRANSPARENT (255) is skipped when drawing.
 *
 * Mario faces RIGHT in the arrays; the renderer mirrors him when
 * he walks left.
 */
#pragma once
#include <stdint.h>

#define SPR_TRANSPARENT 0xFF

/* palette shortcuts for readable art */
#define P_  SPR_TRANSPARENT
#define P_K 0   /* black   */
#define P_R 2   /* red     */
#define P_B 4   /* blue    */
#define P_S 13  /* skin    */
#define P_N 8   /* gray    */
#define P_W 1   /* white   */
#define P_BR 11 /* brown   */
#define P_G 12  /* gold    */
#define P_DG 15 /* dark green */
#define P_O 14 /* orange  */

/* Mario, 12 x 16 */
#define MARIO_W 12
#define MARIO_H 16
static const uint8_t mario_sprite[MARIO_H][MARIO_W] = {
    { P_, P_, P_R, P_R, P_R, P_R, P_R, P_, P_, P_, P_, P_ },
    { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_, P_, P_ },
    { P_, P_K, P_K, P_S, P_K, P_S, P_K, P_K, P_, P_, P_, P_ },
    { P_, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_, P_ },
    { P_, P_S, P_S, P_K, P_S, P_S, P_K, P_S, P_S, P_, P_, P_ },
    { P_, P_, P_S, P_S, P_S, P_S, P_S, P_S, P_, P_, P_, P_ },
    { P_, P_, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_, P_, P_ },
    { P_, P_R, P_R, P_R, P_R, P_R, P_R, P_R, P_, P_, P_, P_ },
    { P_, P_B, P_B, P_B, P_B, P_B, P_B, P_B, P_, P_, P_, P_ },
    { P_, P_B, P_, P_, P_B, P_, P_, P_B, P_, P_B, P_, P_ },
    { P_, P_B, P_, P_, P_B, P_, P_, P_B, P_, P_B, P_, P_ },
    { P_, P_S, P_, P_, P_S, P_, P_, P_S, P_, P_S, P_, P_ },
    { P_, P_S, P_, P_, P_S, P_, P_, P_S, P_, P_S, P_, P_ },
    { P_, P_K, P_, P_, P_K, P_, P_, P_K, P_, P_K, P_, P_ },
    { P_S, P_S, P_, P_, P_, P_, P_, P_, P_, P_S, P_S, P_ },
    { P_S, P_S, P_, P_, P_, P_, P_, P_, P_, P_S, P_S, P_ },
};

/* Goomba-like walker, 12 x 12 (two walk frames) */
#define ENEMY_W 12
#define ENEMY_H 12
static const uint8_t enemy_sprite[2][ENEMY_H][ENEMY_W] = {
    { /* frame 0 */
        { P_, P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_, P_ },
        { P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_ },
        { P_BR, P_K, P_K, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_K, P_K, P_BR },
        { P_BR, P_K, P_K, P_S, P_S, P_S, P_S, P_S, P_S, P_K, P_K, P_BR },
        { P_BR, P_BR, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_BR, P_BR },
        { P_, P_BR, P_BR, P_S, P_S, P_S, P_S, P_S, P_S, P_BR, P_BR, P_ },
        { P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_ },
        { P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR },
        { P_BR, P_BR, P_BR, P_, P_BR, P_BR, P_BR, P_BR, P_, P_BR, P_BR, P_BR },
        { P_BR, P_BR, P_, P_, P_, P_BR, P_BR, P_, P_, P_, P_BR, P_BR },
        { P_BR, P_BR, P_, P_, P_, P_, P_, P_, P_, P_, P_BR, P_BR },
        { P_K, P_K, P_, P_, P_, P_, P_, P_, P_, P_, P_K, P_K },
    },
    { /* frame 1 (feet swapped) */
        { P_, P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_, P_ },
        { P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_ },
        { P_BR, P_K, P_K, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_K, P_K, P_BR },
        { P_BR, P_K, P_K, P_S, P_S, P_S, P_S, P_S, P_S, P_K, P_K, P_BR },
        { P_BR, P_BR, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_S, P_BR, P_BR },
        { P_, P_BR, P_BR, P_S, P_S, P_S, P_S, P_S, P_S, P_BR, P_BR, P_ },
        { P_, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_ },
        { P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR, P_BR },
        { P_BR, P_BR, P_BR, P_, P_BR, P_BR, P_BR, P_BR, P_, P_BR, P_BR, P_BR },
        { P_BR, P_BR, P_, P_, P_BR, P_BR, P_, P_, P_, P_BR, P_BR, P_ },
        { P_BR, P_BR, P_, P_, P_BR, P_BR, P_, P_, P_, P_BR, P_BR, P_ },
        { P_K, P_K, P_, P_, P_K, P_K, P_, P_, P_, P_K, P_K, P_ },
    },
};

/* Coin, 8 x 8 (two frames: coin0 = wide, coin1 = narrow) */
#define COIN_W 8
#define COIN_H 8
static const uint8_t coin_sprite[2][COIN_H][COIN_W] = {
    { /* frame 0 */
        { P_, P_, P_G, P_G, P_G, P_G, P_, P_ },
        { P_, P_G, P_W, P_G, P_G, P_W, P_G, P_ },
        { P_G, P_W, P_, P_, P_, P_, P_W, P_G },
        { P_G, P_W, P_, P_, P_, P_, P_W, P_G },
        { P_G, P_W, P_, P_, P_, P_, P_W, P_G },
        { P_G, P_W, P_, P_, P_, P_, P_W, P_G },
        { P_, P_G, P_W, P_G, P_G, P_W, P_G, P_ },
        { P_, P_, P_G, P_G, P_G, P_G, P_, P_ },
    },
    { /* frame 1 (narrower) */
        { P_, P_, P_, P_, P_, P_, P_, P_ },
        { P_, P_, P_, P_G, P_G, P_, P_, P_ },
        { P_, P_, P_G, P_W, P_W, P_G, P_, P_ },
        { P_, P_, P_G, P_W, P_W, P_G, P_, P_ },
        { P_, P_, P_G, P_W, P_W, P_G, P_, P_ },
        { P_, P_, P_G, P_W, P_W, P_G, P_, P_ },
        { P_, P_, P_, P_G, P_G, P_, P_, P_ },
        { P_, P_, P_, P_, P_, P_, P_, P_ },
    },
};

/* Goal flag, 8 x 14 */
#define FLAG_W 8
#define FLAG_H 14
static const uint8_t flag_sprite[FLAG_H][FLAG_W] = {
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
};
