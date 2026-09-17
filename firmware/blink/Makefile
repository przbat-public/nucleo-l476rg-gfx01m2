# NUCLEO-L476RG + X-NUCLEO-GFX01M2 blink — bare-metal build.
#
# Requirements (see ../README.md for install instructions):
#   arm-none-eabi-gcc  (Arm GNU Toolchain)
#   st-flash           (stlink tools, for flashing)
#
# Targets:
#   make          - build blink.elf / blink.bin
#   make flash    - write blink.bin to the board and reset it
#   make clean    - remove build artifacts

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# -mcpu/-mthumb      : Cortex-M4, Thumb-2
# -mfloat-abi=soft   : no FPU code (the blink uses integer math only)
# -Os                : optimize for size (also fastest here)
# -ffunction-sections/-fdata-sections + --gc-sections: drop unused code
CFLAGS  = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -Os \
          -ffunction-sections -fdata-sections -Wall -Wextra
LDFLAGS = -mcpu=cortex-m4 -mthumb -T linker.ld -nostartfiles \
          -Wl,--gc-sections -Wl,-Map=blink.map

OBJS = startup_l476.o main.o

all: blink.elf blink.bin

blink.elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
	$(SIZE) $@

blink.bin: blink.elf
	$(OBJCOPY) -O binary blink.elf blink.bin

%.o: %.c regs.h
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(CC) $(CFLAGS) -c -o $@ $<

# Note: st-flash does NOT reset the target by itself — hence the extra
# `st-flash reset` (see docs/07-troubleshooting.md).
flash: blink.bin
	st-flash write blink.bin 0x08000000
	st-flash reset

clean:
	rm -f *.o *.elf *.bin *.map

.PHONY: all flash clean
