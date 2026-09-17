/*
 * Joystick scanner — diagnostic build (make scan).
 *
 * Shows the live state of every candidate joystick pin on the display.
 * Two pages, auto-switching every ~4 seconds (PAGE 1/2 in the header).
 * Push the joystick in each direction and note which row turns green —
 * and on which page.
 */
#include "hal.h"
#include "lcd.h"
#include "scan.h"

static void fmt_int(char *buf, int v)
{
    char tmp[8];
    int i = 0;
    if (v == 0) tmp[i++] = '0';
    while (v > 0) { tmp[i++] = (char)('0' + v % 10); v /= 10; }
    while (i > 0) *buf++ = tmp[--i];
    *buf = '\0';
}

int main(void)
{
    system_init();
    lcd_init();
    scan_init();

    uint32_t frame = 0;

    for (;;) {
        frame++;
        int page = (int)((frame / 80) % scan_page_count());   /* ~4 s per page */

        lcd_clear(C_BLACK);
        lcd_text(4, 2, "JOYSTICK SCAN", C_WHITE, C_BLACK, 1);
        char buf[8];
        fmt_int(buf, page + 1);
        lcd_text(150, 2, "PAGE", C_YELLOW, C_BLACK, 1);
        lcd_text(178, 2, buf, C_YELLOW, C_BLACK, 1);

        for (int i = 0; i < scan_count(); i++) {
            int y = 22 + i * 18;
            bool pressed = scan_pressed(page, i);
            uint8_t bg = pressed ? C_GREEN : C_DARK_GRAY;
            lcd_rect(4, y, 235, y + 16, bg);
            lcd_text(10, y + 4, scan_name(page, i), C_BLACK, bg, 1);
        }

        lcd_flush();
        delay_ms(40);
    }
}
