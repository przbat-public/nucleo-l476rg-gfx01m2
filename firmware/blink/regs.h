/*
 * Minimal STM32L476 register definitions for the blink example.
 * Only what a blinking LED needs — everything else lives in the
 * reference manual (RM0351).
 */
#pragma once
#include <stdint.h>

#define MMIO32(addr) (*(volatile uint32_t *)(addr))

/* RCC: reset & clock control */
#define RCC_BASE    0x40021000UL
#define RCC_CR      MMIO32(RCC_BASE + 0x00)   /* HSI16ON=8, HSI16RDY=10, PLLON=24, PLLRDY=25 */
#define RCC_CFGR    MMIO32(RCC_BASE + 0x08)   /* SW=1:0, SWS=3:2 */
#define RCC_PLLCFGR MMIO32(RCC_BASE + 0x0C)   /* PLLSRC=1:0, PLLN=14:8, PLLREN=24 */
#define RCC_AHB2ENR MMIO32(RCC_BASE + 0x4C)   /* GPIOAEN=0 */

/* FLASH access control */
#define FLASH_ACR   MMIO32(0x40022000UL)      /* LATENCY=2:0, PRFTEN=8, ICEN=9, DCEN=10 */

/* GPIOA */
#define GPIOA_BASE  0x48000000UL
#define GPIO_MODER   MMIO32(GPIOA_BASE + 0x00)  /* 2 bits/pin: 01 = output */
#define GPIO_BSRR    MMIO32(GPIOA_BASE + 0x18)  /* write 1<<n to SET pin n */
#define GPIO_BRR     MMIO32(GPIOA_BASE + 0x28)  /* write 1<<n to RESET pin n */

void delay_ms(uint32_t ms);
