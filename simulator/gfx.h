#ifndef GFX_H
#define GFX_H

#include <stdbool.h>
#include <stdint.h>

#define STRIP_H 6

#define RGB(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

void gfx_begin_strip(int top);
void gfx_flush_strip(void);
bool gfx_in_strip(int y, int h);

void fill_rect(int x, int y, int w, int h, uint16_t color);
void draw_rect(int x, int y, int w, int h, uint16_t color);
void fill_circle(int cx, int cy, int r, uint16_t color);
void draw_circle(int cx, int cy, int r, uint16_t color);
void fill_round_rect(int x, int y, int w, int h, int r, uint16_t color);
void fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color);
void print_at(int x, int y, int size, uint16_t color, const char *text);
int text_width(const char *text, int size);

#endif
