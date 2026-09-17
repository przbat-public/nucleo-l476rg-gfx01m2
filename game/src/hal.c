/*
 * hal.c — Hardware Abstraction Layer implementation.
 *
 * This is the ONLY file in the project that touches hardware registers.
 * Everything above it (lcd.c, input.c, game.c) speaks the clean API from
 * hal.h. To port the game to another board, rewrite this file only.
 *
 * Register-level details are documented in the main demo (firmware/main)
 * and in docs/03-hello-world.md.
 */
#include "hal.h"

/* --- register definitions (kept private to this file) --- */
#define MMIO32(addr) (*(volatile uint32_t *)(addr))

#define RCC_BASE     0x40021000UL
#define RCC_CR       MMIO32(RCC_BASE + 0x00)
#define RCC_CFGR     MMIO32(RCC_BASE + 0x08)
#define RCC_PLLCFGR  MMIO32(RCC_BASE + 0x0C)
#define RCC_AHB2ENR  MMIO32(RCC_BASE + 0x4C)
#define RCC_APB2ENR  MMIO32(RCC_BASE + 0x60)

#define FLASH_ACR    MMIO32(0x40022000UL)

#define GPIO(port)        (0x48000000UL + 0x400UL * (port))
#define GPIO_MODER(p)     MMIO32(GPIO(p) + 0x00)
#define GPIO_OSPEEDR(p)   MMIO32(GPIO(p) + 0x08)
#define GPIO_PUPDR(p)     MMIO32(GPIO(p) + 0x0C)
#define GPIO_IDR(p)       MMIO32(GPIO(p) + 0x10)
#define GPIO_BSRR(p)      MMIO32(GPIO(p) + 0x18)
#define GPIO_BRR(p)       MMIO32(GPIO(p) + 0x28)
#define GPIO_AFRL(p)      MMIO32(GPIO(p) + 0x20)

#define SPI1_CR1   MMIO32(0x40013000UL + 0x00)
#define SPI1_CR2   MMIO32(0x40013000UL + 0x04)
#define SPI1_SR    MMIO32(0x40013000UL + 0x08)
#define SPI1_DR8   (*(volatile uint8_t *)(0x40013000UL + 0x0C))

/* ------------------------------------------------------------------ */
/* Clock: HSI16 -> PLL x10 /2 = 80 MHz (see firmware/main for details) */
/* ------------------------------------------------------------------ */
static void clock_init(void)
{
    FLASH_ACR = 4u | (1u << 8) | (1u << 9) | (1u << 10);

    RCC_CR |= (1u << 8);                             /* HSI16ON  */
    while (!(RCC_CR & (1u << 10))) {}                /* HSI16RDY */
    RCC_PLLCFGR = (2u << 0) | (10u << 8) | (1u << 24);
    RCC_CR |= (1u << 24);                            /* PLLON    */
    while (!(RCC_CR & (1u << 25))) {}                /* PLLRDY   */
    RCC_CFGR = (3u << 0);                            /* SW = PLL */
    while ((RCC_CFGR & (3u << 2)) != (3u << 2)) {}
}

/* After enabling a peripheral clock, wait a moment before touching it. */
static void clk_sync(void)
{
    __asm volatile ("dsb sy" ::: "memory");
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

void system_init(void)
{
    clock_init();

    /* GPIOA + GPIOB + GPIOC clocks */
    RCC_AHB2ENR |= (1u << 0) | (1u << 1) | (1u << 2);
    clk_sync(); (void)RCC_AHB2ENR;

    /* SPI1 clock */
    RCC_APB2ENR |= (1u << 12);
    clk_sync(); (void)RCC_APB2ENR;

    /* SPI1: master, 8-bit, mode 0, BR=/2 -> 40 MHz, software NSS */
    SPI1_CR1 = (1u << 2) | (1u << 8) | (1u << 9);
    SPI1_CR2 = (7u << 8);
    SPI1_CR1 |= (1u << 6);

    /* Display pins (morpho/Arduino route on the GFX01M2 shield):
     *   PA5/PA6/PA7 = SPI1 SCK/MISO/MOSI (AF5)
     *   PA9  = LCD CS   (output)
     *   PB10 = LCD DC   (output)
     *   PA1  = LCD RST  (output)
     */
    GPIO_MODER(PORT_A) &= ~((3u << 10) | (3u << 12) | (3u << 14));
    GPIO_MODER(PORT_A) |= (2u << 10) | (2u << 12) | (2u << 14);
    GPIO_AFRL(PORT_A)  &= ~((0xFu << 20) | (0xFu << 24) | (0xFu << 28));
    GPIO_AFRL(PORT_A)  |= (5u << 20) | (5u << 24) | (5u << 28);
    GPIO_OSPEEDR(PORT_A) |= (3u << 10) | (3u << 12) | (3u << 14);

    gpio_output(PORT_A, 9);    /* CS  */
    gpio_output(PORT_B, 10);   /* DC  */
    gpio_output(PORT_A, 1);    /* RST */
    gpio_set(PORT_A, 9);
    gpio_set(PORT_A, 1);
}

void delay_ms(uint32_t ms)
{
    while (ms--) {
        for (volatile uint32_t i = 0; i < 12000; i++) { }
    }
}

void gpio_output(uint8_t port, uint8_t pin)
{
    GPIO_MODER(port) &= ~(3u << (pin * 2));
    GPIO_MODER(port) |= (1u << (pin * 2));
    GPIO_OSPEEDR(port) |= (3u << (pin * 2));
}

void gpio_set(uint8_t port, uint8_t pin)
{
    GPIO_BSRR(port) = (1u << pin);
}

void gpio_clear(uint8_t port, uint8_t pin)
{
    GPIO_BRR(port) = (1u << pin);
}

void gpio_input_pullup(uint8_t port, uint8_t pin)
{
    GPIO_MODER(port) &= ~(3u << (pin * 2));        /* input      */
    GPIO_PUPDR(port) &= ~(3u << (pin * 2));        /* pull-up    */
    GPIO_PUPDR(port) |= (1u << (pin * 2));
}

bool gpio_read(uint8_t port, uint8_t pin)
{
    return (GPIO_IDR(port) >> pin) & 1u;
}

void spi_write(const uint8_t *buf, uint32_t n)
{
    while (n--) {
        while (!(SPI1_SR & (1u << 1))) {}          /* wait TXE */
        SPI1_DR8 = *buf++;
        while (SPI1_SR & (1u << 7)) {}             /* wait BSY (not a hot path) */
    }
}
