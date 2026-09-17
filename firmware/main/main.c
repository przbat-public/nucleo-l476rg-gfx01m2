/*
 * NUCLEO-L476RG + X-NUCLEO-GFX01M2 demo — bare-metal C, no HAL, no vendor libs.
 *
 * This file contains only the "board bring-up" part:
 *   1. clock_init()  - HSI16 -> PLL -> 80 MHz system clock
 *   2. gpio_init()   - SPI1 pins, LCD control pins, UART TX pin
 *   3. spi_init()    - SPI1 master, 40 MHz, software chip-select
 *   4. uart_init()   - USART2 @ 115200 baud (the ST-LINK virtual COM port)
 *   5. delay_ms()    - simple busy-wait (good enough for LCD timings)
 *
 * The actual graphics live in st7789.c (LCD driver) and fx.c (effects).
 *
 * Build:   make
 * Flash:   st-flash write demo.bin 0x08000000 && st-flash reset
 */
#include "regs.h"
#include "st7789.h"
#include "fx.h"
#include <stdint.h>

/*
 * On STM32L4, after enabling a peripheral clock you must wait a moment
 * (a DSB + a dummy read) before touching that peripheral's registers,
 * otherwise the first writes can be silently lost (RM0351).
 * This cost us an hour of debugging — see docs/07-troubleshooting.md.
 */
static inline void rcc_sync(void)
{
    __asm volatile ("dsb sy" ::: "memory");
}

/* ------------------------------------------------------------------ */
/* Clock: HSI16 -> PLL x10 /2 = 80 MHz                                 */
/*                                                                     */
/* We deliberately do NOT use the MSI oscillator. On at least one      */
/* L476RG the MSIRANGE switch silently failed (the core stayed at      */
/* 4 MHz) — see docs/07-troubleshooting.md. HSI16+PLL is rock solid.   */
/* ------------------------------------------------------------------ */
static void clock_init(void)
{
    /* Flash: 4 wait states for 80 MHz + prefetch + instruction/data cache. */
    FLASH_ACR = 4u | (1u << 8) | (1u << 9) | (1u << 10);

    RCC_CR |= (1u << 8);                           /* HSI16ON  — enable 16 MHz RC */
    while (!(RCC_CR & (1u << 10))) {}              /* wait for HSI16RDY */

    /* PLL: source=HSI16 (2), VCO x10 (N=10), output /2 (R=00) -> 80 MHz */
    RCC_PLLCFGR = (2u << 0) | (10u << 8) | (1u << 24);
    RCC_CR |= (1u << 24);                          /* PLLON */
    while (!(RCC_CR & (1u << 25))) {}              /* wait for PLLRDY */

    RCC_CFGR = (3u << 0);                          /* SYSCLK source = PLL */
    while ((RCC_CFGR & (3u << 2)) != (3u << 2)) {} /* wait for SWS = PLL */

    /* AHB, APB1 and APB2 all /1 -> core 80 MHz, APB2 (SPI1) 80 MHz. */
}

/* ------------------------------------------------------------------ */
/* GPIO                                                                 */
/* ------------------------------------------------------------------ */
static void gpio_init(void)
{
    RCC_AHB2ENR |= (1u << 0) | (1u << 1);          /* GPIOA + GPIOB clocks */
    rcc_sync(); (void)RCC_AHB2ENR;

    /* PA5/PA6/PA7 = SPI1 SCK/MISO/MOSI (alternate function 5) */
    GPIO_MODER(0) &= ~((3u << 10) | (3u << 12) | (3u << 14));
    GPIO_MODER(0) |= (2u << 10) | (2u << 12) | (2u << 14);
    GPIO_AFRL(0)  &= ~((0xFu << 20) | (0xFu << 24) | (0xFu << 28));
    GPIO_AFRL(0)  |= (5u << 20) | (5u << 24) | (5u << 28);
    GPIO_OSPEEDR(0) |= (3u << 10) | (3u << 12) | (3u << 14);

    /* LCD control outputs (push-pull):
     *   PA9  = CS  (chip select, active low)
     *   PA1  = RST (reset,      active low)
     *   PB10 = DC  (data/command: 0=command, 1=data)
     */
    GPIO_MODER(0) &= ~((3u << 18) | (3u << 2));
    GPIO_MODER(0) |= (1u << 18) | (1u << 2);
    GPIO_MODER(1) &= ~(3u << 20);
    GPIO_MODER(1) |= (1u << 20);
    GPIO_OSPEEDR(0) |= (3u << 18) | (3u << 2);
    GPIO_OSPEEDR(1) |= (3u << 20);
    GPIO_BSRR(0) = (1u << 9) | (1u << 1);          /* CS=1, RST=1 (idle) */

    /* PA2 = USART2_TX (alternate function 7) — logs over the VCP */
    GPIO_MODER(0) &= ~(3u << 4);
    GPIO_MODER(0) |= (2u << 4);
    GPIO_AFRL(0)  &= ~(0xFu << 8);
    GPIO_AFRL(0)  |= (7u << 8);
}

/* ------------------------------------------------------------------ */
/* SPI1: master, 8-bit, mode 0, 40 MHz (APB2=80 MHz, BR=/2)           */
/* ------------------------------------------------------------------ */
static void spi_init(void)
{
    RCC_APB2ENR |= (1u << 12);                     /* SPI1EN */
    rcc_sync(); (void)RCC_APB2ENR;

    /* MSTR (master), BR=000 (/2 -> 40 MHz), SSI+SSM (software NSS) */
    SPI1_CR1 = (1u << 2) | (1u << 8) | (1u << 9);
    SPI1_CR2 = (7u << 8);                          /* DS = 8 bits */
    SPI1_CR1 |= (1u << 6);                         /* SPE — enable */
}

/* ------------------------------------------------------------------ */
/* USART2 @ 115200 8N1 — appears on your PC as the ST-LINK VCP         */
/* (a serial port like /dev/cu.usbmodem* on macOS).                    */
/* ------------------------------------------------------------------ */
static void uart_init(void)
{
    RCC_APB1ENR1 |= (1u << 17);                    /* USART2EN */
    rcc_sync(); (void)RCC_APB1ENR1;
    USART2_BRR = (694u << 4) | 7u;                 /* 80 MHz / 115200 = 694.44 */
    USART2_CR1 = (1u << 0) | (1u << 3) | (1u << 2); /* UE | TE | RE */
}

static void uart_putc(char c)
{
    while (!(USART2_ISR & (1u << 7))) {}           /* wait TXE */
    USART2_TDR8 = (uint8_t)c;
}

static void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}

/* ------------------------------------------------------------------ */
/* Busy-wait delay. Fine for LCD power-up timings (needs ~1 ms         */
/* accuracy, nothing more). 12000 iterations ≈ 1 ms @ 80 MHz.         */
/* ------------------------------------------------------------------ */
void delay_ms(uint32_t ms)
{
    while (ms--) {
        for (volatile uint32_t i = 0; i < 12000; i++) { }
    }
}

/* ------------------------------------------------------------------ */
/* main: bring the board up, then run the show forever                 */
/* ------------------------------------------------------------------ */
int main(void)
{
    clock_init();
    gpio_init();
    spi_init();
    uart_init();

    uart_puts("\r\n=== L476RG + X-NUCLEO-GFX01M2 demo ===\r\n");
    uart_puts("HSI16 + PLL 80 MHz, SPI1 40 MHz, USART2 115200\r\n");

    lcd_init();
    uart_puts("LCD init OK - show starts\r\n");

    fx_run();   /* plasma + starfield, loops forever */
}
