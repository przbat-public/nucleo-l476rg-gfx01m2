/*
 * scan.h — joystick pin scanner (two pages, auto-cycling).
 */
#pragma once
#include <stdbool.h>

void scan_init(void);
int  scan_count(void);              /* pins per page */
int  scan_page_count(void);
const char *scan_name(int page, int i);
bool scan_pressed(int page, int i);
