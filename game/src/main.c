/*
 * Mini-Mario — entry point.
 *
 * Board bring-up is one line (system_init), the display is one line
 * (lcd_init), input is one line (input_init) — then the game takes
 * over. Nothing hardware-specific below this file.
 */
#include "hal.h"
#include "lcd.h"
#include "input.h"
#include "game.h"

int main(void)
{
    system_init();
    lcd_init();
    input_init();

    game_run();   /* never returns */
    return 0;
}
