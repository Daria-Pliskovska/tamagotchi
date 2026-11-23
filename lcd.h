#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define LCD_W 320
#define LCD_H 240

void lcd_init(void);
void lcd_push_rows(int top, int rows, const uint16_t *pixels);

#endif
