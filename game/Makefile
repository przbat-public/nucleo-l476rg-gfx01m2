# Mini-Mario — build
#
#   make          - the game
#   make flash    - flash the game
#   make scan     - the joystick pin scanner (diagnostic)
#   make scanflash- flash the scanner
#   make clean

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

CFLAGS  = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -Os -g \
          -ffunction-sections -fdata-sections -Wall -Wextra -Isrc
LDFLAGS = -mcpu=cortex-m4 -mthumb -T linker.ld -nostartfiles \
          -Wl,--gc-sections -Wl,-Map=game.map

GAME_OBJS = startup_l476.o src/main.o src/hal.o src/lcd.o \
            src/input.o src/game.o src/font5x7.o
SCAN_OBJS = startup_l476.o src/scan_main.o src/hal.o src/lcd.o \
            src/scan.o src/font5x7.o

all: game.elf game.bin

game.elf: $(GAME_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(GAME_OBJS)
	$(SIZE) $@

game.bin: game.elf
	$(OBJCOPY) -O binary game.elf game.bin

scan: scan.elf scan.bin

scan.elf: $(SCAN_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(SCAN_OBJS)
	$(SIZE) $@

scan.bin: scan.elf
	$(OBJCOPY) -O binary scan.elf scan.bin

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(CC) $(CFLAGS) -c -o $@ $<

flash: game.bin
	st-flash write game.bin 0x08000000
	st-flash reset

scanflash: scan.bin
	st-flash write scan.bin 0x08000000
	st-flash reset

clean:
	rm -f *.o src/*.o *.elf *.bin *.map

.PHONY: all flash scan scanflash clean
