/*
 * Blink — the classic "hello world" of embedded development.
 *
 * Toggles the green user LED (LD2, connected to PA5 on the
 * NUCLEO-L476RG) about twice per second.
 *
 * This example is deliberately tiny: one file + the same startup/
 * linker/clock plumbing as the main demo. It is a great first thing
 * to flash to prove your toolchain and the ST-LINK USB connection
 * work before moving on to the display.
 *
 * Build:   make
 * Flash:   st-flash write blink.bin 0x08000000 && st-flash reset
 */
#include "regs.h"

/* ------------------------------------------------------------------ */
/* Clock: HSI16 -> PLL x10 /2 = 80 MHz (see the main demo's main.c    */
/* for the full explanation).                                          */
/* ------------------------------------------------------------------ */
static void clock_init(void)
{
    FLASH_ACR = 4u | (1u << 8) | (1u << 9) | (1u << 10); /* 4 WS + caches */

    RCC_CR |= (1u << 8);                            /* HSI16ON */
    while (!(RCC_CR & (1u << 10))) {}               /* HSI16RDY */
    RCC_PLLCFGR = (2u << 0) | (10u << 8) | (1u << 24);
    RCC_CR |= (1u << 24);                           /* PLLON */
    while (!(RCC_CR & (1u << 25))) {}               /* PLLRDY */
    RCC_CFGR = (3u << 0);                           /* SYSCLK = PLL */
    while ((RCC_CFGR & (3u << 2)) != (3u << 2)) {}  /* SWS = PLL */
}

/* ------------------------------------------------------------------ */
/* PA5 as a push-pull output — it drives LED LD2.                      */
/* ------------------------------------------------------------------ */
static void gpio_init(void)
{
    RCC_AHB2ENR |= (1u << 0);                       /* GPIOA clock */
    __asm volatile ("dsb sy" ::: "memory");         /* see main demo */
    (void)RCC_AHB2ENR;

    GPIO_MODER &= ~(3u << 10);                   /* PA5: mode 01 = output */
    GPIO_MODER |= (1u << 10);
}

/* ------------------------------------------------------------------ */
/* Busy-wait delay, ~1 ms per unit at 80 MHz.                          */
/* ------------------------------------------------------------------ */
void delay_ms(uint32_t ms)
{
    while (ms--) {
        for (volatile uint32_t i = 0; i < 12000; i++) { }
    }
}

int main(void)
{
    clock_init();
    gpio_init();

    for (;;) {
        GPIO_BSRR = (1u << 5);    /* LED on  (BSRR sets the pin)  */
        delay_ms(250);
        GPIO_BRR  = (1u << 5);    /* LED off (BRR resets the pin) */
        delay_ms(250);
    }
}
