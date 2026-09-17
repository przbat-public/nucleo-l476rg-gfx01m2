# 07 · Troubleshooting

Every problem hit while building this project, with symptoms and fixes.
Each one of these cost real debugging time — this file exists so you
don't have to pay it again.

## 1. `Failed to enter SWD mode` (but ST-LINK is visible, voltage is fine)

```
$ st-info --probe
Failed to enter SWD mode
  version:    V3J6 / V2J41S27
  chipid:     0x000
```

**The #1 cause on this project: the X-NUCLEO-GFX01M2 shield is mounted
the wrong way round.** The shield's connectors are not symmetric — turned
180° they land on the wrong pins and take the SWD lines with them.
Mounted correctly, SWD works perfectly with the shield on.

- Target voltage reading fine + no SWD response is the signature.
- Holding the reset button during probing does *not* help — this is not
  a firmware-sleep problem, it's the wiring.
- Fix: mount the shield the right way round (compare with the photos on
  the ST product page) and everything lights up.

Secondary causes of the same symptom:

- **USB hub in the path.** Some ST-LINKs (especially V3) misbehave
  through hubs — Apple's multiport adapters and USB 1.1 hubs are the
  worst offenders. Plug the board directly into the computer.
- **Loose micro-USB socket.** The ST-LINK enumerates, drops, re-enumerates
  (you see a flickering device / stale entries in the USB tree). Replug
  firmly or try another cable.
- **Another tool holds the ST-LINK.** OpenOCD and `st-flash` open the
  device exclusively; close one before using the other.

## 2. `st-flash` reports success but the old program still runs

`st-flash write` does **not** reset the target afterwards. The new
firmware sits in flash while the old one keeps executing with its old
peripheral state — which looks exactly like "my changes did nothing".

```bash
st-flash reset        # always after st-flash write
```

The repo's `make flash` already does both.

## 3. Everything works but is ~12× slower than expected

On the L476RG used for this project the MSI oscillator range switch
silently failed: the range bits in `RCC_CR` *read back as written*, yet
the core stayed at the 4 MHz reset frequency. The animation ran at
0.5 fps instead of ~17 fps, with zero errors anywhere.

- **How to measure:** enable the DWT cycle counter and count cycles per
  frame — if a 2.5-second frame contains 10 million cycles, your core
  runs at ~4 MHz, whatever the registers say.
- **Fix:** don't fight the MSI. Switch to HSI16 → PLL (see
  [03 · Hello world](03-hello-world.md)); it is deterministic on every
  silicon.
- **Don't** write `RCC_ICSCR` to force the range — on the L476 it is
  read-only and the write raises a data-access fault.

## 4. First register writes to a freshly enabled peripheral get lost

Classic STM32L4 trap: enable the peripheral clock, then write its
registers immediately — and the first writes vanish. The peripheral
needs a moment to come out of reset.

```c
RCC_APB1ENR1 |= (1u << 17);              /* enable USART2 clock  */
__asm volatile ("dsb sy" ::: "memory");  /* wait                 */
(void)RCC_APB1ENR1;                      /* dummy read           */
USART2_CR1 = ...;                        /* now safe             */
```

The repo wraps this in `rcc_sync()` in `main.c`. Symptom if you forget:
the UART stays silent and its `CR1` reads back as `0`.

## 5. Text on the display is mirrored

The `MADCTL` register (command `0x36`) controls orientation. The `MX`
bit (0x40) mirrors horizontally, `MY` (0x80) vertically, `MV` (0x20)
rotates by swapping X/Y, `BGR` (0x08) swaps red/blue.

- Mirrored left-right → clear the `MX` bit.
- Upside down → clear `MY`.
- Rotated 90° → toggle `MV` (and swap width/height).
- Red and blue swapped → toggle `BGR`.

This repo uses `MADCTL = 0x08` (BGR only, no mirror).

## 6. The display stays black

Work through this list:

1. **Shield orientation** — see issue 1 (wrong way round also kills the
   image).
2. **RST pin** — the panel needs the reset pulse. On the L476RG it is
   **PA1** (not PB2 as some older docs suggest).
3. **CS/DC pins** — on the L476RG: **CS = PA9, DC = PB10**. Getting
   these wrong means the panel never sees a valid command.
4. **SPI polarity** — mode 0 (CPOL=0, CPHA=0).
5. **Init delays** — `SLPOUT` needs ~120 ms; if your busy-wait is
   miscalibrated (e.g. you changed the clock and kept the old loop
   constant), the panel may still be asleep.
6. **Brightness of your content** — a single dim pixel on a 2.2" panel
   is invisible. This project's first starfield drew 1-pixel stars with
   linear brightness and looked like a black screen.

## 7. The VCP (virtual COM port) stays silent

On some boards the ST-LINK VCP route is disconnected or broken even
though USART2 is configured correctly (solder bridges SB13/SB14 on the
NUCLEO-64). Before debugging your UART code:

1. Check the baud rate on **both** ends (115200 8N1).
2. Verify `USART2_CR1` and `BRR` via GDB (see
   [06 · Debugging](06-debugging.md)).
3. Check whether the port is the *right* one (`ls /dev/cu.usbmodem*`)
   — with several boards connected there may be more than one.
4. As a last resort, accept it: SWD debugging works regardless, and the
   demo needs no UART at all.

## 8. `arm-none-eabi-gcc` install asks for a password (macOS cask)

The ARM toolchain cask runs a `sudo` installer. Password-free alternative:
install the same `.pkg` into your home directory — exact commands in
[01 · Toolchain setup](01-toolchain-setup.md).

## Quick reference: healthy values

```
st-info --probe          chipid 0x415, dev-type STM32L47x_L48x
RCC_CR after clock_init  0x01000163-ish (PLLRDY + PLLON + HSI16 set)
SPI1_CR1                 0x0344  (SPE, MSTR, BR=/2, SSI, SSM)
USART2_CR1               0x000D  (UE | RE | TE)
USART2_BRR               0x2B47  (694.44 @ 80 MHz / 115200)
```

---

Back to the [README](../README.md)
