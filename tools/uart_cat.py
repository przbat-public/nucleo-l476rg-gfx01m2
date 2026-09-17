#!/usr/bin/env python3
"""Odczyt portu VCP z wymuszonym baud (macOS). Uzycie: uart_cat.py <port> [sekundy]"""
import os, sys, termios, time

port = sys.argv[1] if len(sys.argv) > 1 else "/dev/cu.usbmodem102"
secs = float(sys.argv[2]) if len(sys.argv) > 2 else 8.0

fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
a = termios.tcgetattr(fd)
a[0] &= ~0x1F          # wyczysc bity baud
a[0] |= 0x0300 | 0x8000 | 0x0800   # CS8 | CLOCAL | CREAD
a[0] &= ~(0x1000 | 0x0100)         # bez parzystosci, 1 bit stopu
a[4] = termios.B115200
a[5] = termios.B115200
a[3] &= ~(termios.ICANON | termios.ECHO | termios.ISIG)
termios.tcsetattr(fd, termios.TCSANOW, a)

end = time.time() + secs
buf = b""
while time.time() < end:
    try:
        chunk = os.read(fd, 4096)
        if chunk:
            buf += chunk
    except BlockingIOError:
        time.sleep(0.05)

os.close(fd)
sys.stdout.buffer.write(buf)
