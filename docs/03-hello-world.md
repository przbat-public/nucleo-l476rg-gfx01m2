# 03 · Hello world (blink)

Every embedded journey starts with an LED. This chapter walks through
[`firmware/blink/`](../firmware/blink) — the smallest possible program that
runs on the L476RG and proves your whole toolchain works.

The green user LED (LD2) is wired to **PA5** on the NUCLEO-L476RG.

## What a bare-metal program needs

A C program on "big" computers starts because an OS loads it. On a
microcontroller you provide everything yourself:

### 1. A vector table (`startup_l476.s`)

The first 8 bytes of flash are special: word 0 = initial stack pointer,
word 1 = reset handler address. The CPU reads them right after reset.
Everything else in the table is exception handlers — for blink they all
point at `Default_Handler`, which just spins.

### 2. A linker script (`linker.ld`)

Tells the linker where flash (1 MB @ `0x08000000`) and SRAM
(96 KB @ `0x20000000`) are, and where each section goes:

| Section | Contents | Where |
|---|---|---|
| `.isr_vector` | the vector table | start of flash |
| `.text` | code + constants | flash |
| `.data` | initialized globals | stored in flash, copied to SRAM at boot |
| `.bss` | zeroed globals | SRAM only |

### 3. A reset handler

Copies `.data` from flash to SRAM, zeroes `.bss`, then calls `main()`.
That is exactly what a tiny OS would do before handing you the CPU.

### 4. A clock tree (`clock_init()`)

The chip starts at 16 MHz (HSI16 RC oscillator). `clock_init()` switches
to HSI16 → PLL ×10 ÷2 = **80 MHz**:

```c
FLASH_ACR = 4u | (1u<<8) | (1u<<9) | (1u<<10);  /* 4 wait states + caches */
RCC_CR |= (1u<<8);                  /* HSI16 on              */
while (!(RCC_CR & (1u<<10))) {}     /* wait HSI16RDY         */
RCC_PLLCFGR = (2u<<0)|(10u<<8)|(1u<<24);
RCC_CR |= (1u<<24);                 /* PLL on                */
while (!(RCC_CR & (1u<<25))) {}     /* wait PLLRDY           */
RCC_CFGR = (3u<<0);                 /* SYSCLK = PLL          */
```

Two details worth understanding:

- **Flash wait states.** At 80 MHz the flash memory cannot answer in one
  cycle, so the access controller is told to wait 4 cycles (`LATENCY`).
  The caches (`ICEN`/`DCEN`) and prefetch (`PRFTEN`) hide most of that
  latency for code.
- **Why not the MSI oscillator?** The L476 has a fine MSI, but on the
  board used for this project the MSI range switch silently failed —
  see [07 · Troubleshooting](07-troubleshooting.md). HSI16+PLL is
  boring but deterministic.

### 5. GPIO (`gpio_init()`)

To blink, PA5 must become an output. GPIO registers (RM0351, chapter 8):

```c
RCC_AHB2ENR |= (1u<<0);            /* clock for GPIOA      */
GPIO_MODER &= ~(3u<<10);           /* PA5: mode 01 = output */
GPIO_MODER |= (1u<<10);
```

`GPIO_MODER` packs 2 bits per pin (`00` input, `01` output, `10` alternate
function, `11` analog). Setting and resetting a pin is a single write to
`BSRR` (set) or `BRR` (reset) — no read-modify-write races.

### 6. The loop

```c
for (;;) {
    GPIO_BSRR = (1u<<5);   /* LED on  */
    delay_ms(250);
    GPIO_BRR  = (1u<<5);   /* LED off */
    delay_ms(250);
}
```

`delay_ms` is a calibrated busy loop (`12000` iterations ≈ 1 ms @ 80 MHz).
Good enough for an LED and for LCD power-up timings; a real project would
use a timer interrupt.

## Build and flash

```bash
cd firmware/blink
make
make flash          # = st-flash write + st-flash reset
```

`make` produces:

```
arm-none-eabi-size blink.elf
   text    data     bss     dec     hex  filename
    540       0       0     540     21c  blink.elf
```

Half a kilobyte. The green LED now blinks ~2 times per second.

## Reading the firmware

```bash
arm-none-eabi-objdump -d blink.elf | less    # disassembly
arm-none-eabi-readelf -a blink.elf | less    # sections, symbols
```

The disassembly of `main` is the best way to *see* what your C turned
into — check how the compiler implements the busy loop and the GPIO
writes.

---

Next: [04 · The display](04-display.md)
