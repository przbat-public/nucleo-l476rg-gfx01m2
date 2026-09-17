# 01 · Toolchain setup

You need three tools: a cross-compiler, a flasher, and (optionally) a
debugger. Everything here is free and open source.

## 1. Arm GNU Toolchain (`arm-none-eabi-gcc`)

The compiler that produces Cortex-M code.

### macOS (Homebrew)

```bash
brew install --cask gcc-arm-embedded
```

The cask installer asks for your password. If you want a password-free
install (e.g. you don't have admin rights), the same official package can be
installed into your home directory:

```bash
brew fetch --cask gcc-arm-embedded
installer -pkg ~/Library/Caches/Homebrew/downloads/*arm-none-eabi*.pkg \
           -target CurrentUserHomeDirectory
ln -sf ~/Applications/ArmGNUToolchain/*/arm-none-eabi/bin/* /opt/homebrew/bin/
```

### Linux (Debian/Ubuntu)

```bash
sudo apt install gcc-arm-none-eabi
```

### Verify

```bash
arm-none-eabi-gcc --version    # should print a version like 13.x / 15.x
```

## 2. stlink tools (`st-flash`)

A small command-line flasher that talks to the on-board ST-LINK over USB.

### macOS

```bash
brew install stlink
```

### Linux

```bash
sudo apt install stlink-tools
```

### Verify

```bash
st-flash --version
```

## 3. OpenOCD + GDB (for debugging)

OpenOCD is a debug server; GDB connects to it. The compiler package already
contains `arm-none-eabi-gdb`.

### macOS

```bash
brew install openocd
```

### Linux

```bash
sudo apt install openocd
```

### Verify

```bash
openocd --version
arm-none-eabi-gdb --version
```

## 4. `make`

macOS ships it; on Linux: `sudo apt install make`.

---

Next: [02 · First connection](02-first-connection.md)
