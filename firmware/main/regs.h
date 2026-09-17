/*
 * Minimal STM32L476 (RM0351) register definitions — just what this
 * project needs. Addresses and bit positions are taken from the
 * reference manual; for a full set use the official CMSIS device
 * header (stm32l476xx.h).
 */
#pragma once
#include <stdint.h>

#define MMIO32(addr) (*(volatile uint32_t *)(addr))

/* --- RCC: reset & clock control --- */
#define RCC_BASE    0x40021000UL
#define RCC_CR      MMIO32(RCC_BASE + 0x00)   /* HSI16ON=8, HSI16RDY=10, PLLON=24, PLLRDY=25 */
#define RCC_CFGR    MMIO32(RCC_BASE + 0x08)   /* SW=1:0, SWS=3:2, HPRE=7:4, PPRE1=10:8, PPRE2=13:11 */
#define RCC_PLLCFGR MMIO32(RCC_BASE + 0x0C)   /* PLLSRC=1:0, PLLM=6:4, PLLN=14:8, PLLREN=24, PLLR=26:25 */
#define RCC_AHB2ENR MMIO32(RCC_BASE + 0x4C)   /* GPIOAEN=0, GPIOBEN=1, GPIOCEN=2 */
#define RCC_APB1ENR1 MMIO32(RCC_BASE + 0x58)  /* USART2EN=17 */
#define RCC_APB2ENR MMIO32(RCC_BASE + 0x60)   /* SYSCFGEN=0, SPI1EN=12 */

/* --- FLASH: access control --- */
#define FLASH_ACR   MMIO32(0x40022000UL)      /* LATENCY=2:0, PRFTEN=8, ICEN=9, DCEN=10 */

/* --- GPIO: port A..B (port offset 0x400 each) --- */
#define GPIO(port)  (0x48000000UL + 0x400UL * (port))
#define GPIO_MODER(p)  MMIO32(GPIO(p) + 0x00)  /* 2 bits/pin: 00=in 01=out 10=AF 11=analog */
#define GPIO_OTYPER(p) MMIO32(GPIO(p) + 0x04)
#define GPIO_OSPEEDR(p) MMIO32(GPIO(p) + 0x08) /* 2 bits/pin: 11 = very high */
#define GPIO_PUPDR(p)  MMIO32(GPIO(p) + 0x0C)
#define GPIO_IDR(p)    MMIO32(GPIO(p) + 0x10)
#define GPIO_ODR(p)    MMIO32(GPIO(p) + 0x14)
#define GPIO_BSRR(p)   MMIO32(GPIO(p) + 0x18)  /* write 1<<n to set pin n */
#define GPIO_BRR(p)    MMIO32(GPIO(p) + 0x28)  /* write 1<<n to reset pin n */
#define GPIO_AFRL(p)   MMIO32(GPIO(p) + 0x20)  /* AF for pins 0-7,  4 bits each */
#define GPIO_AFRH(p)   MMIO32(GPIO(p) + 0x24)  /* AF for pins 8-15, 4 bits each */

/* --- SPI1 --- */
#define SPI1_BASE   0x40013000UL
#define SPI1_CR1    MMIO32(SPI1_BASE + 0x00)  /* CPHA=0, CPOL=1, MSTR=2, BR=5:3, SPE=6, SSI=8, SSM=9 */
#define SPI1_CR2    MMIO32(SPI1_BASE + 0x04)  /* DS=11:8 (0b0111 = 8 bit) */
#define SPI1_SR     MMIO32(SPI1_BASE + 0x08)  /* RXNE=0, TXE=1, BSY=7 */
#define SPI1_DR8    (*(volatile uint8_t *)(SPI1_BASE + 0x0C))

/* --- USART2 --- */
#define USART2_BASE 0x40004400UL
#define USART2_CR1  MMIO32(USART2_BASE + 0x00) /* UE=0, RE=2, TE=3 */
#define USART2_BRR  MMIO32(USART2_BASE + 0x0C) /* baud = fck/(mantissa + frac/16) */
#define USART2_ISR  MMIO32(USART2_BASE + 0x1C) /* RXNE=5, TC=6, TXE=7 */
#define USART2_TDR8 (*(volatile uint8_t *)(USART2_BASE + 0x28))
#define USART2_RDR8 (*(volatile uint8_t *)(USART2_BASE + 0x24))

/* --- delay (defined in main.c) --- */
void delay_ms(uint32_t ms);

/*
 * SPI1 byte transmit — inline because it is the animation hot path.
 *
 * We wait only for TXE (transmit buffer empty), NOT for BSY.
 * TXE already guarantees that the previous byte has left the shift
 * register, so consecutive bytes pipeline at full speed.
 * Before raising /CS, call spi_flush() to drain the last byte.
 */
static inline void spi_putc(uint8_t b)
{
    while (!(SPI1_SR & (1u << 1))) {}   /* wait TXE */
    SPI1_DR8 = b;
}

static inline void spi_flush(void)
{
    while (!(SPI1_SR & (1u << 1))) {}   /* TXE — buffer empty   */
    while (SPI1_SR & (1u << 7)) {}      /* BSY — shift finished */
}
