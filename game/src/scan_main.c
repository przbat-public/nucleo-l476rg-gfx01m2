/*
 * Joystick scanner — diagnostic build (make scan).
 *
 * Shows the live state of every candidate joystick pin on the display.
 * Push the joystick in each direction and note which row turns green.
 */
#include "hal.h"
#include "lcd.h"
#include "scan.h"

int main(void)
{
    system_init();
    lcd_init();
    scan_init();

    for (;;) {
        lcd_clear(C_BLACK);
        lcd_text(4, 2, "JOYSTICK SCAN - push directions", C_WHITE, C_BLACK, 1);

        for (int i = 0; i < scan_count(); i++) {
            int y = 22 + i * 18;
            bool pressed = scan_pressed(i);
            uint8_t bg = pressed ? C_GREEN : C_DARK_GRAY;
            lcd_rect(4, y, 235, y + 16, bg);
            lcd_text(10, y + 4, scan_name(i), C_BLACK, bg, 1);
        }

        lcd_flush();
        delay_ms(40);
    }
}
