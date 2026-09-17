/*
 * scan.c — joystick pin scanner implementation.
 */
#include "scan.h"
#include "hal.h"
#include <stdint.h>

typedef struct {
    uint8_t port;
    uint8_t pin;
    const char *name;
} pin_candidate_t;

static const pin_candidate_t candidates[] = {
    { PORT_A,  0, "PA0" },
    { PORT_A,  4, "PA4" },
    { PORT_A,  8, "PA8" },
    { PORT_A, 15, "PA15" },
    { PORT_B,  0, "PB0" },
    { PORT_B,  1, "PB1" },
    { PORT_B,  2, "PB2" },
    { PORT_B,  4, "PB4" },
    { PORT_B, 12, "PB12" },
    { PORT_B, 13, "PB13" },
    { PORT_C,  0, "PC0" },
    { PORT_C,  2, "PC2" },
    { PORT_C,  7, "PC7" },
    { PORT_C,  8, "PC8" },
    { PORT_C,  9, "PC9" },
    { PORT_C, 13, "PC13 (B1 button)" },
};

#define CAND_COUNT ((int)(sizeof(candidates) / sizeof(candidates[0])))

/* Release the JTAG pins (PB3/PB4/PA15) for GPIO use, keep SWD. */
static void jtag_release(void)
{
    *(volatile uint32_t *)(0x40021000UL + 0x60) |= (1u << 0);
    __asm volatile ("dsb sy" ::: "memory");
    volatile uint32_t *cfgr1 = (volatile uint32_t *)(0x40010000UL);
    *cfgr1 = (*cfgr1 & ~(7u << 24)) | (2u << 24);
}

void scan_init(void)
{
    jtag_release();
    for (int i = 0; i < CAND_COUNT; i++)
        gpio_input_pullup(candidates[i].port, candidates[i].pin);
}

int scan_count(void) { return CAND_COUNT; }
const char *scan_name(int i) { return candidates[i].name; }

/* Switches are active-low: pressed = pin reads 0. */
bool scan_pressed(int i)
{
    return !gpio_read(candidates[i].port, candidates[i].pin);
}
