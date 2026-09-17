/*
 * input.c — joystick + button input.
 *
 * THE PIN MAP (single source of truth, per UM2750 group 8 = L476RG):
 *
 *   DIR_LEFT   = PA4
 *   DIR_RIGHT  = PB4
 *   DIR_UP     = PB13
 *   DIR_DOWN   = PA15
 *   DIR_CENTER = PC13   (TEMPORARY: the blue USER button B1 — the
 *                        shield's center press is still being verified
 *                        with the joystick scanner; B1 acts as the
 *                        action/jump button meanwhile)
 *
 * All switches are active-low (pressed = pin reads 0).
 * To change a pin, edit this table and nothing else.
 */
#include "input.h"
#include "hal.h"
#include <stdint.h>

/* -------------------- game input -------------------- */

typedef struct { uint8_t port; uint8_t pin; } pin_t;

static const pin_t joy_map[DIR_COUNT] = {
    [DIR_UP]     = { PORT_B, 13 },
    [DIR_DOWN]   = { PORT_A, 15 },
    [DIR_LEFT]   = { PORT_A, 4  },
    [DIR_RIGHT]  = { PORT_B, 4  },
    [DIR_CENTER] = { PORT_C, 13 },   /* USER button B1 */
};

/*
 * Release the JTAG pins (PB3/PB4/PA15) for GPIO use while keeping
 * SWD (PA13/PA14) alive for the debugger.
 */
static void jtag_release(void)
{
    *(volatile uint32_t *)(0x40021000UL + 0x60) |= (1u << 0);  /* SYSCFGEN */
    __asm volatile ("dsb sy" ::: "memory");
    volatile uint32_t *cfgr1 = (volatile uint32_t *)(0x40010000UL);
    *cfgr1 = (*cfgr1 & ~(7u << 24)) | (2u << 24);              /* JTAG off, SWD on */
}

void input_init(void)
{
    jtag_release();
    for (int d = 0; d < DIR_COUNT; d++)
        gpio_input_pullup(joy_map[d].port, joy_map[d].pin);
}

bool input_held(dir_t d)
{
    if (d >= DIR_COUNT) return false;
    return !gpio_read(joy_map[d].port, joy_map[d].pin);
}

/*
 * Frame-based debounced press: a direction must read pressed for two
 * consecutive game frames to be reported once. No timers involved —
 * the game loop calls this once per frame. Holding a direction does
 * NOT auto-repeat (menus want single presses; movement uses
 * input_held directly).
 */
dir_t input_read(void)
{
    static int     last = -1;
    static uint8_t stable = 0;

    dir_t pressed = DIR_COUNT;
    for (int d = 0; d < DIR_COUNT; d++) {
        if (input_held((dir_t)d)) { pressed = (dir_t)d; break; }
    }

    if ((int)pressed == last) {
        stable++;
    } else {
        last = (int)pressed;
        stable = 1;
    }

    if (pressed != DIR_COUNT && stable == 2)
        return pressed;            /* newly confirmed press */
    return DIR_COUNT;
}
