/*
 * input.h — joystick + button input.
 *
 * Game-facing API:
 *   input_init()      — configure joystick pins
 *   input_read()      — a single, debounced press (for menus)
 *   input_held(dir)   — raw held state (for movement)
 *
 * The pin mapping lives in ONE table at the top of input.c —
 * change it there and nowhere else.
 */
#pragma once
#include <stdbool.h>

typedef enum {
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_CENTER,
    DIR_COUNT
} dir_t;

void input_init(void);
bool input_held(dir_t d);
dir_t input_read(void);   /* debounced single press, DIR_COUNT = none */
