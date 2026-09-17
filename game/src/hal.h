/*
 * hal.h — Hardware Abstraction Layer.
 *
 * The whole point of this layer: game code NEVER touches a register.
 * Ports and pins are passed as plain numbers; the mapping to hardware
 * registers happens only inside hal.c.
 *
 *   system_init()          — clocks (HSI16+PLL -> 80 MHz), SPI1, display pins
 *   delay_ms()             — busy-wait delay
 *   gpio_output/set/clear  — digital outputs
 *   gpio_input_pullup/read — digital inputs (joystick switches)
 *   spi_write()            — raw SPI transmit (used by lcd.c only)
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>

/* GPIO ports */
#define PORT_A 0
#define PORT_B 1
#define PORT_C 2

void system_init(void);
void delay_ms(uint32_t ms);

void gpio_output(uint8_t port, uint8_t pin);
void gpio_set(uint8_t port, uint8_t pin);
void gpio_clear(uint8_t port, uint8_t pin);
void gpio_input_pullup(uint8_t port, uint8_t pin);
bool gpio_read(uint8_t port, uint8_t pin);

void spi_write(const uint8_t *buf, uint32_t n);
