# 06 · Debugging

A blinking LED and a silent display tell you little. The board's ST-LINK
gives you a full debugger over the same USB cable — this is where the
real bugs get caught.

## The stack: OpenOCD + GDB

```
arm-none-eabi-gdb  ──(TCP:3333)──>  OpenOCD  ──(USB)──>  ST-LINK  ──(SWD)──>  MCU
```

OpenOCD translates GDB's requests into SWD traffic; `arm-none-eabi-gdb`
is the debugger from your toolchain.

## Start a session

```bash
# terminal 1 — the debug server
openocd -f interface/stlink.cfg -f target/stm32l4x.cfg
```

```
Info : STLINK V2J41M27 (API v2) VID:PID 0483:3752
Info : Target voltage: 3.246597
Info : [stm32l4x.cpu] Cortex-M4 r0p1 processor detected
Info : [stm32l4x.cpu] target has 6 breakpoints, 4 watchpoints
Info : Listening on port 3333 for gdb connections
```

```bash
# terminal 2 — the debugger
cd firmware/main
arm-none-eabi-gdb demo.elf
(gdb) target extended-remote :3333
(gdb) monitor reset halt      # reset and freeze before main()
(gdb) break main
(gdb) continue
```

Build with `-g` for source-level symbols (the repo's Makefile already
does).

## The daily toolkit

```gdb
step                 # one source line
next                 # like step, over function calls
print variable       # inspect a C variable
print/x $pc          # registers: $pc, $sp, $lr, $r0..$r12
info registers
x/16wx 0x20000000   # dump memory
monitor mdw 0x40021000 2     # memory via OpenOCD
monitor reset halt           # reset the chip
continue / quit
```

## Reading peripheral registers

The most useful trick for bare-metal work: read the *actual* hardware
registers and compare with what your code was supposed to write:

```gdb
print/x *(unsigned int*)0x40021000    # RCC_CR
print/x *(unsigned int*)0x40004400    # USART2_CR1
print/x *(unsigned int*)0x40013000    # SPI1_CR1
```

This is how the project's two nastiest bugs were found:

1. **The clock never switched.** The firmware ran 12× slower than it
   should. Reading `RCC_CR` showed the range bits *were* written, yet a
   cycle-count measurement (DWT) proved the core ran at ~4 MHz. Fix:
   switch to HSI16+PLL.
2. **Peripheral writes lost.** `USART2_CR1` read back as `0` right after
   the init code had written `0xD`. Cause: touching a peripheral
   register too soon after enabling its clock. Fix: `dsb` + dummy read.

## Catching faults

When the MCU hits an exception it lands in `Default_Handler` (a `b .`
spin). To see *why*:

```gdb
monitor halt
print/x *(unsigned int*)0xE000ED28    # CFSR — fault status
print/x *(unsigned int*)0xE000ED2C    # HFSR — hard fault status
print/x *(unsigned int*)0xE000ED38    # BFAR — faulting address
print/x $pc                           # where it died
```

Useful CFSR bits: `0x100` = instruction fetch fault (jumped to a garbage
address — often a corrupted return address), `0x200` = precise data bus
fault (read/wrote an address that doesn't exist — often an unclocked
peripheral), `0x20000` = data access violation (e.g. writing a
read-only register).

## Watchpoints — catching the writer

Hardware watchpoints stop the CPU the moment a specific address is
written (the M4 has 4 of them):

```gdb
watch *(unsigned int*)0x20000100
continue
```

If a variable "magically" changes, a watchpoint names the guilty
instruction immediately.

## Practical notes

- **One tool at a time.** `st-flash` and OpenOCD both open the ST-LINK
  exclusively — close OpenOCD before flashing.
- **`st-flash` does not reset the target** after writing; `make flash`
  chains `st-flash reset` for you.
- **The shield does not block SWD** when mounted correctly — no need to
  remove it for debugging.
- Live-register inspection beats printf on a board whose UART is not
  wired the way you expect.

---

Next: [07 · Troubleshooting](07-troubleshooting.md)
