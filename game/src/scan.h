/*
 * scan.h — joystick pin scanner (diagnostic tool).
 *
 * Configures every candidate joystick pin as an input with a pull-up
 * and reports their live state. Push the joystick in each direction
 * and watch which pin reacts — that pin IS that direction.
 */
#pragma once
#include <stdbool.h>

void scan_init(void);
int  scan_count(void);
const char *scan_name(int i);
bool scan_pressed(int i);
