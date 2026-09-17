# 02 · First connection

The NUCLEO board carries an **ST-LINK** programmer/debugger chip. One USB
cable gives you three things at once:

1. **SWD** — flashing and debugging the target MCU
2. **Virtual COM port (VCP)** — a serial port wired to the MCU's USART2
3. power for the board

> Plug the cable into the **ST-LINK connector** (the one near the
> ST-LINK chip, labeled "ST-LINK" / "USB ST-LINK"), not the target USB.

## Detect the board

```bash
st-info --probe
```

Healthy output for the L476RG:

```
Found 1 stlink programmers
  version:    V2J41S27
  serial:     066CFF505271754867074825
  flash:      1048576 (pagesize: 2048)
  sram:       98304
  chipid:     0x415            <- 0x415 = STM32L47x/L48x
  dev-type:   STM32L47x_L48x
```

`chipid 0x415` is the debugger *reading the MCU over SWD* — this single
line proves that USB, the ST-LINK and the target are all alive.

### If it fails

- `Couldn't find any ST-Link devices` → cable/port problem. Try another
  cable, another USB port, or plug the board directly (no hubs — some
  ST-LINKs misbehave through hubs, especially USB 1.1 ones).
- `Failed to enter SWD mode` → the ST-LINK is visible but cannot reach the
  MCU. On this project the #1 cause is the **display shield mounted the
  wrong way round** — see [07 · Troubleshooting](07-troubleshooting.md).
- A list of stale devices on macOS: replug the cable.

## The virtual COM port

```bash
# macOS / Linux — list serial ports
ls /dev/cu.usbmodem* /dev/ttyACM* 2>/dev/null
```

The firmware logs to USART2 (PA2) at 115200 baud. You can read it with
`screen /dev/cu.usbmodemXXXX 115200` (quit: Ctrl-A then K) or with the
helper in this repo: `python3 tools/uart_cat.py /dev/cu.usbmodemXXXX 10`.

> On the board used to develop this repo the VCP stayed silent even with
> USART2 correctly configured (solder bridges SB13/SB14 route it) — worth
> knowing before you chase your own tail. SWD debugging works regardless.

## What about the shield?

You can flash and debug **with the shield mounted** — as long as it is
mounted the right way round (see the pinout in the README). The next
chapters assume the shield is on the board.

---

Next: [03 · Hello world (blink)](03-hello-world.md)
