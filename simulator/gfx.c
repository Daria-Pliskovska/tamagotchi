#include <math.h>
#include <string.h>
#include "gfx.h"
#include "lcd.h"
#include "font.h"

static uint16_t strip[LCD_W * STRIP_H];
static int strip_top;
static int strip_rows;

void gfx_begin_strip(int top)
{
    strip_top = top;
    strip_rows = LCD_H - top < STRIP_H ? LCD_H - top : STRIP_H;
}

void gfx_flush_strip(void)
{
    lcd_push_rows(strip_top, strip_rows, strip);
}

bool gfx_in_strip(int y, int h)
{
    return y + h > strip_top && y < strip_top + strip_rows;
}

static void hline(int x0, int x1, int y, uint16_t color)
{
    if (y < strip_top || y >= strip_top + strip_rows) {
        return;
    }
    if (x0 < 0) {
        x0 = 0;
    }
    if (x1 > LCD_W - 1) {
        x1 = LCD_W - 1;
    }
    uint16_t *row = strip + (y - strip_top) * LCD_W;
    for (int x = x0; x <= x1; x++) {
        row[x] = color;
    }
}

static void set_pixel(int x, int y, uint16_t color)
{
    if (x >= 0 && x < LCD_W) {
        hline(x, x, y, color);
    }
}

void fill_rect(int x, int y, int w, int h, uint16_t color)
{
    int first = y > strip_top ? y : strip_top;
    int last = y + h < strip_top + strip_rows ? y + h : strip_top + strip_rows;
    for (int row = first; row < last; row++) {
        hline(x, x + w - 1, row, color);
    }
}

void draw_rect(int x, int y, int w, int h, uint16_t color)
{
    fill_rect(x, y, w, 1, color);
    fill_rect(x, y + h - 1, w, 1, color);
    fill_rect(x, y, 1, h, color);
    fill_rect(x + w - 1, y, 1, h, color);
}

void fill_circle(int cx, int cy, int r, uint16_t color)
{
    for (int dy = -r; dy <= r; dy++) {
        int y = cy + dy;
        if (y < strip_top || y >= strip_top + strip_rows) {
            continue;
        }
        int half = (int)sqrt((double)(r * r - dy * dy));
        hline(cx - half, cx + half, y, color);
    }
}

void draw_circle(int cx, int cy, int r, uint16_t color)
{
    int x = r;
    int y = 0;
    int error = 1 - r;

    while (x >= y) {
        set_pixel(cx + x, cy + y, color);
        set_pixel(cx - x, cy + y, color);
        set_pixel(cx + x, cy - y, color);
        set_pixel(cx - x, cy - y, color);
        set_pixel(cx + y, cy + x, color);
        set_pixel(cx - y, cy + x, color);
        set_pixel(cx + y, cy - x, color);
        set_pixel(cx - y, cy - x, color);
        y++;
        if (error < 0) {
            error += 2 * y + 1;
        } else {
            x--;
            error += 2 * (y - x) + 1;
        }
    }
}

void fill_round_rect(int x, int y, int w, int h, int r, uint16_t color)
{
    fill_rect(x + r, y, w - 2 * r, h, color);
    fill_rect(x, y + r, w, h - 2 * r, color);
    fill_circle(x + r, y + r, r, color);
    fill_circle(x + w - r - 1, y + r, r, color);
    fill_circle(x + r, y + h - r - 1, r, color);
    fill_circle(x + w - r - 1, y + h - r - 1, r, color);
}

static void edge_cross(int xa, int ya, int xb, int yb, int y, int *left, int *right)
{
    if (y < (ya < yb ? ya : yb) || y > (ya > yb ? ya : yb)) {
        return;
    }
    int x0 = xa;
    int x1 = xb;
    if (ya != yb) {
        x0 = xa + (long)(y - ya) * (xb - xa) / (yb - ya);
        x1 = x0;
    }
    if (x0 < *left) {
        *left = x0;
    }
    if (x1 > *right) {
        *right = x1;
    }
    if (x1 < *left) {
        *left = x1;
    }
    if (x0 > *right) {
        *right = x0;
    }
}

void fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color)
{
    for (int y = strip_top; y < strip_top + strip_rows; y++) {
        int left = LCD_W;
        int right = -1;
        edge_cross(x0, y0, x1, y1, y, &left, &right);
        edge_cross(x1, y1, x2, y2, y, &left, &right);
        edge_cross(x2, y2, x0, y0, y, &left, &right);
        if (left <= right) {
            hline(left, right, y, color);
        }
    }
}

static void draw_char(int x, int y, int size, uint16_t color, char c)
{
    if (c < FONT_FIRST || c >= FONT_FIRST + FONT_CHARS) {
        return;
    }
    if (!gfx_in_strip(y, FONT_H * size)) {
        return;
    }
    const uint8_t *glyph = FONT + (c - FONT_FIRST) * FONT_W;
    for (int col = 0; col < FONT_W; col++) {
        uint8_t bits = pgm_read_byte(glyph + col);
        for (int row = 0; row < FONT_H; row++) {
            if (bits & (1 << row)) {
                fill_rect(x + col * size, y + row * size, size, size, color);
            }
        }
    }
}

void print_at(int x, int y, int size, uint16_t color, const char *text)
{
    for (int i = 0; text[i] != '\0'; i++) {
        draw_char(x + i * (FONT_W + 1) * size, y, size, color, text[i]);
    }
}

int text_width(const char *text, int size)
{
    return (int)strlen(text) * (FONT_W + 1) * size;
}
