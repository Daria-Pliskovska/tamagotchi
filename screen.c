#include <Arduino.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "gfx.h"
#include "lcd.h"
#include "screen.h"

#define SCREEN_W LCD_W
#define SCREEN_H LCD_H
#define BAR_H 18
#define HORIZON_Y 150

#define PX 7
#define SMALL_PX 4
#define BUNNY_W 16
#define BUNNY_H 20
#define ICON_SIZE 8
#define BUNNY_X ((SCREEN_W - BUNNY_W * PX) / 2)
#define BUNNY_Y 82

#define BAR_CELLS 5
#define BAR_EXTRA_CELLS 3

#define COLOR_SKY      RGB(120, 190, 220)
#define COLOR_HILLS    RGB(60, 140, 100)
#define COLOR_GRASS    RGB(100, 190, 90)
#define COLOR_PATH     RGB(150, 170, 100)
#define COLOR_CLOUD    RGB(235, 245, 240)
#define COLOR_SUN      RGB(250, 210, 90)
#define COLOR_TRUNK    RGB(120, 70, 40)
#define COLOR_TREE     RGB(40, 120, 60)
#define COLOR_LCD      RGB(200, 235, 215)
#define COLOR_INK      RGB(40, 70, 60)
#define COLOR_BAR      RGB(90, 90, 110)
#define COLOR_OUTLINE  RGB(40, 70, 140)
#define COLOR_FUR      RGB(130, 190, 245)
#define COLOR_PINK     RGB(245, 160, 190)
#define COLOR_EYE      RGB(20, 20, 30)
#define COLOR_WHITE    RGB(255, 255, 255)
#define COLOR_TEAR     RGB(80, 160, 255)
#define COLOR_DROOL    RGB(110, 180, 250)
#define COLOR_POOP     RGB(140, 90, 50)
#define COLOR_CARROT   RGB(240, 140, 40)
#define COLOR_LEAF     RGB(60, 160, 60)
#define COLOR_YELLOW   RGB(250, 210, 70)
#define COLOR_HAT      RGB(230, 70, 120)
#define COLOR_BLUE     RGB(70, 130, 230)
#define COLOR_GREEN    RGB(90, 190, 90)
#define COLOR_ANGRY    RGB(220, 50, 50)

typedef enum {
    MODE_NONE,
    MODE_SCENE,
    MODE_STATS,
    MODE_OVER
} ScreenMode;

static ScreenMode current_mode = MODE_NONE;
static long last_key = -1;
static char last_message[40];
static bool last_ai_enabled;

static const char *BUNNY[BUNNY_H] = {
    "...oo......oo...",
    "..obbo....obbo..",
    "..obpo....opbo..",
    "..obpo....opbo..",
    "..obpo....opbo..",
    "..obbo....obbo..",
    ".oobboooooobboo.",
    ".obbbbbbbbbbbbo.",
    "obbbbbbbbbbbbbbo",
    "obbeebbbbbbeebbo",
    "obbewbbbbbbwebbo",
    "obpbbbboobbbbpbo",
    ".obbbbbbbbbbbbo.",
    "..oooooooooooo..",
    "..obbbbbbbbbbo..",
    ".obbbwwwwwwbbbo.",
    "obobbwwwwwwbbobo",
    "..obbbbbbbbbbo..",
    "..obbbo..obbbo..",
    "..oooo....oooo..",
};

static const char *BUNNY_FAT[BUNNY_H] = {
    "...oo......oo...",
    "..obbo....obbo..",
    "..obpo....opbo..",
    "..obpo....opbo..",
    "..obpo....opbo..",
    "..obbo....obbo..",
    ".oobboooooobboo.",
    ".obbbbbbbbbbbbo.",
    "obbbbbbbbbbbbbbo",
    "obbeebbbbbbeebbo",
    "obbewbbbbbbwebbo",
    "obppbbboobbbppbo",
    "obbbbbbbbbbbbbbo",
    "obbbbbbbbbbbbbbo",
    "obbbbwwwwwwbbbbo",
    "obbwwwwwwwwwwbbo",
    "obbwwwwwwwwwwbbo",
    ".obbbwwwwwwbbbo.",
    "..obbbo..obbbo..",
    "..oooo....oooo..",
};

static const char *CARROT_UP[ICON_SIZE] = {
    ".....g.g",
    "......gg",
    ".....cc.",
    "....ccc.",
    "...ccc..",
    "..ccc...",
    ".cc.....",
    "c.......",
};

static const char *CARROT_HELD[ICON_SIZE] = {
    "c.......",
    ".cc.....",
    ".ccc....",
    "..ccc...",
    "...ccc..",
    "....cc.g",
    "......gg",
    ".....g.g",
};

static const char *POOP[ICON_SIZE] = {
    "....#...",
    "...##...",
    "..####..",
    "..#..#..",
    ".######.",
    ".#....#.",
    "########",
    "........",
};

static bool palette_color(char c, uint16_t *color)
{
    switch (c) {
    case 'o':
        *color = COLOR_OUTLINE;
        return true;
    case 'b':
        *color = COLOR_FUR;
        return true;
    case 'p':
        *color = COLOR_PINK;
        return true;
    case 'e':
        *color = COLOR_EYE;
        return true;
    case 'w':
        *color = COLOR_WHITE;
        return true;
    case 'c':
        *color = COLOR_CARROT;
        return true;
    case 'g':
        *color = COLOR_LEAF;
        return true;
    case '#':
        *color = COLOR_POOP;
        return true;
    default:
        return false;
    }
}

static void draw_cell(int x, int y, int col, int row, int size, uint16_t color)
{
    fill_rect(x + col * size, y + row * size, size, size, color);
}

static void draw_sprite(const char **rows, int height, int x, int y, int size)
{
    for (int row = 0; row < height; row++) {
        if (!gfx_in_strip(y + row * size, size)) {
            continue;
        }
        for (int col = 0; rows[row][col] != '\0'; col++) {
            uint16_t color;
            if (palette_color(rows[row][col], &color)) {
                draw_cell(x, y, col, row, size, color);
            }
        }
    }
}

static void draw_background(void)
{
    fill_rect(0, BAR_H, SCREEN_W, HORIZON_Y - BAR_H, COLOR_SKY);
    fill_circle(292, 42, 12, COLOR_SUN);
    fill_round_rect(16, 40, 76, 18, 9, COLOR_CLOUD);
    fill_round_rect(36, 30, 40, 20, 10, COLOR_CLOUD);
    fill_round_rect(196, 62, 64, 16, 8, COLOR_CLOUD);

    for (int i = 0; i < 8; i++) {
        int height = 6 + (i % 3) * 4;
        fill_rect(i * 40, HORIZON_Y - height, 40, height, COLOR_HILLS);
    }
    fill_rect(0, HORIZON_Y, SCREEN_W, SCREEN_H - HORIZON_Y, COLOR_GRASS);
    fill_triangle(155, HORIZON_Y + 10, 165, HORIZON_Y + 10, 200, SCREEN_H, COLOR_PATH);
    fill_triangle(155, HORIZON_Y + 10, 120, SCREEN_H, 200, SCREEN_H, COLOR_PATH);

    fill_rect(28, HORIZON_Y - 22, 5, 20, COLOR_TRUNK);
    fill_circle(30, HORIZON_Y - 30, 12, COLOR_TREE);
}

static void set_eyes(int x, int y, uint16_t top, uint16_t bottom)
{
    int eye_cols[] = {3, 4, 11, 12};
    for (int i = 0; i < 4; i++) {
        draw_cell(x, y, eye_cols[i], 9, PX, top);
        draw_cell(x, y, eye_cols[i], 10, PX, bottom);
    }
}

static void draw_open_mouth(int x, int y)
{
    for (int col = 7; col <= 8; col++) {
        draw_cell(x, y, col, 11, PX, COLOR_OUTLINE);
        draw_cell(x, y, col, 12, PX, COLOR_OUTLINE);
    }
}

static void draw_drool(int x, int y, float time)
{
    int length = 2 + (int)(time * 3) % 3;
    int drool_x = x + 8 * PX + 2;
    int drool_y = y + 12 * PX;
    fill_rect(drool_x, drool_y, SMALL_PX, SMALL_PX * length, COLOR_DROOL);
    fill_circle(drool_x + SMALL_PX / 2, drool_y + SMALL_PX * length, 3, COLOR_DROOL);
}

static void draw_sleeping(int x, int y, float time)
{
    set_eyes(x, y, COLOR_FUR, COLOR_OUTLINE);
    draw_drool(x, y, time);
    int shift = (int)(time * 2) % 3 * 3;
    print_at(x + BUNNY_W * PX, y + 40 - shift, 2, COLOR_INK, "z");
    print_at(x + BUNNY_W * PX + 14, y + 12 - shift, 3, COLOR_INK, "Z");
}

static void draw_eating(int x, int y, float time)
{
    if ((int)(time * 5) % 2 == 0) {
        draw_open_mouth(x, y);
    }
    draw_sprite(CARROT_HELD, ICON_SIZE, x + 8 * PX + 4, y + 11 * PX, SMALL_PX);
}

static void draw_party_hat(int x, int y)
{
    int center = x + 8 * PX;
    int base = y + 6 * PX;
    int rows = 10;
    int top = base - rows * SMALL_PX;

    for (int k = 0; k < rows; k++) {
        uint16_t stripe = (k / 2) % 2 == 0 ? COLOR_HAT : COLOR_YELLOW;
        fill_rect(center - k * 2 - 2, top + k * SMALL_PX, k * 4 + 4, SMALL_PX, stripe);
    }
    fill_rect(center - 4, top - 8, 8, 8, COLOR_WHITE);
}

static void draw_confetti(float time)
{
    uint16_t colors[] = {COLOR_HAT, COLOR_YELLOW, COLOR_BLUE, COLOR_GREEN};
    for (int i = 0; i < 14; i++) {
        int cx = 10 + (i * 67) % (SCREEN_W - 20);
        int cy = BAR_H + 10 + (int)fmod(time * 50.0 + i * 29.0, SCREEN_H - BAR_H - 30);
        fill_rect(cx, cy, 5, 5, colors[i % 4]);
    }
}

static void draw_playing(int x, int y, float time)
{
    draw_cell(x, y, 7, 11, PX, COLOR_FUR);
    draw_cell(x, y, 8, 11, PX, COLOR_FUR);
    draw_cell(x, y, 6, 11, PX, COLOR_OUTLINE);
    draw_cell(x, y, 9, 11, PX, COLOR_OUTLINE);
    draw_cell(x, y, 7, 12, PX, COLOR_OUTLINE);
    draw_cell(x, y, 8, 12, PX, COLOR_OUTLINE);
    draw_party_hat(x, y);
    draw_confetti(time);
}

static void draw_thought_cloud(int x, int y)
{
    int cx = 266;
    int cy = 64;
    int parts[4][3] = {{-14, 4, 15}, {14, 4, 15}, {0, -9, 17}, {0, 11, 13}};

    for (int i = 0; i < 4; i++) {
        fill_circle(cx + parts[i][0], cy + parts[i][1], parts[i][2] + 2, COLOR_INK);
    }
    for (int i = 0; i < 4; i++) {
        fill_circle(cx + parts[i][0], cy + parts[i][1], parts[i][2], COLOR_WHITE);
    }
    fill_circle(x + BUNNY_W * PX + 4, y + 7 * PX, 4, COLOR_WHITE);
    draw_circle(x + BUNNY_W * PX + 4, y + 7 * PX, 4, COLOR_INK);
    fill_circle(x + BUNNY_W * PX + 18, y + 4 * PX, 6, COLOR_WHITE);
    draw_circle(x + BUNNY_W * PX + 18, y + 4 * PX, 6, COLOR_INK);

    draw_sprite(CARROT_UP, ICON_SIZE, cx - 16, cy - 16, SMALL_PX);
}

static void draw_hungry(int x, int y, float time)
{
    draw_open_mouth(x, y);
    draw_drool(x, y, time);
    draw_thought_cloud(x, y);
}

static void draw_sleepy(int x, int y, float time)
{
    bool yawning = fmod(time, 3.0) < 1.0;

    if (yawning) {
        set_eyes(x, y, COLOR_FUR, COLOR_OUTLINE);
        for (int col = 6; col <= 9; col++) {
            for (int row = 11; row <= 13; row++) {
                draw_cell(x, y, col, row, PX, COLOR_OUTLINE);
            }
        }
        draw_cell(x, y, 7, 12, PX, COLOR_PINK);
        draw_cell(x, y, 8, 12, PX, COLOR_PINK);
        print_at(x + BUNNY_W * PX + 6, y + 50, 2, COLOR_INK, "yawn");
    } else {
        int eye_cols[] = {3, 4, 11, 12};
        for (int i = 0; i < 4; i++) {
            draw_cell(x, y, eye_cols[i], 9, PX, COLOR_FUR);
        }
    }
}

static void draw_sad(int x, int y, float time)
{
    int row = 11 + (int)(time * 4) % 3;
    draw_cell(x, y, 3, row, PX, COLOR_TEAR);
    draw_cell(x, y, 12, row, PX, COLOR_TEAR);
    draw_open_mouth(x, y);
}

static void draw_tantrum(int x, int y, float time)
{
    draw_cell(x, y, 3, 8, PX, COLOR_OUTLINE);
    draw_cell(x, y, 12, 8, PX, COLOR_OUTLINE);
    for (int col = 6; col <= 9; col++) {
        for (int row = 11; row <= 13; row++) {
            draw_cell(x, y, col, row, PX, COLOR_OUTLINE);
        }
    }
    draw_cell(x, y, 7, 12, PX, COLOR_ANGRY);
    draw_cell(x, y, 8, 12, PX, COLOR_ANGRY);

    int row = 11 + (int)(time * 6) % 3;
    draw_cell(x, y, 2, row, PX, COLOR_TEAR);
    draw_cell(x, y, 13, row, PX, COLOR_TEAR);

    if ((int)(time * 4) % 2 == 0) {
        print_at(x - 50, y + 10, 3, COLOR_ANGRY, "NO!!");
    }
}

static int bunny_x(const Pet *pet, Mood mood, float time)
{
    int x = BUNNY_X;
    if (mood == MOOD_TANTRUM) {
        x += (int)(pet_escape_progress(pet) * 120.0f);
        x += (int)(sin(time * 40.0) * 3.0);
    }
    return x;
}

static void draw_scene(const Pet *pet, Mood mood, int x, int y, float time)
{
    draw_background();
    if (pet->clean < 50.0f) {
        draw_sprite(POOP, ICON_SIZE, 60, 196, SMALL_PX);
    }
    draw_sprite(pet_is_fat(pet) ? BUNNY_FAT : BUNNY, BUNNY_H, x, y, PX);

    switch (mood) {
    case MOOD_SLEEPING:
        draw_sleeping(x, y, time);
        break;
    case MOOD_TANTRUM:
        draw_tantrum(x, y, time);
        break;
    case MOOD_EATING:
        draw_eating(x, y, time);
        break;
    case MOOD_PLAYING:
        draw_playing(x, y, time);
        break;
    case MOOD_HUNGRY:
        draw_hungry(x, y, time);
        break;
    case MOOD_SLEEPY:
        draw_sleepy(x, y, time);
        break;
    case MOOD_SAD:
        draw_sad(x, y, time);
        break;
    default:
        break;
    }
}

static void draw_status(const Assistant *ai)
{
    const char *label = ai->enabled ? "AI ON" : "AI OFF";

    fill_rect(0, 0, SCREEN_W, BAR_H, COLOR_BAR);
    print_at(4, 2, 2, COLOR_WHITE, ai->message);
    print_at(SCREEN_W - text_width(label, 2) - 4, 2, 2,
             ai->enabled ? COLOR_GREEN : COLOR_ANGRY, label);
}

static void draw_bar(int y, const char *name, float value)
{
    print_at(10, y, 2, COLOR_INK, name);
    for (int i = 0; i < BAR_CELLS; i++) {
        int cx = 110 + i * 24;
        if (value > i * 20) {
            fill_rect(cx, y, 20, 14, COLOR_INK);
        } else {
            draw_rect(cx, y, 20, 14, COLOR_INK);
        }
    }
    for (int i = BAR_CELLS; i < BAR_CELLS + BAR_EXTRA_CELLS; i++) {
        if (value > i * 20) {
            fill_rect(110 + i * 24, y, 20, 14, COLOR_CARROT);
        }
    }
}

static void draw_stats(const Pet *pet, const Assistant *ai)
{
    char line[40];

    fill_rect(0, BAR_H, SCREEN_W, SCREEN_H - BAR_H, COLOR_LCD);
    draw_bar(26, "HUNGER", pet->hunger);
    draw_bar(50, "DIAPER", pet->clean);
    draw_bar(74, "FUN", pet->fun);
    draw_bar(98, "ENERGY", pet->energy);
    draw_bar(122, "HEALTH", pet->health);

    snprintf(line, sizeof(line), "AI memory (%s):", ai->enabled ? "ON" : "OFF");
    print_at(10, 152, 2, COLOR_INK, line);
    for (int i = 0; i < ACTION_SLEEP; i++) {
        snprintf(line, sizeof(line), "%s %dx, %dm", action_name((Action)i),
                 ai->times_done[i], assistant_minutes_left(ai, pet, (Action)i));
        print_at(10, 176 + i * 20, 2, COLOR_INK, line);
    }
}

static void draw_game_over(const Pet *pet)
{
    const char *title = pet->ran_away ? "RAN AWAY!" : "GAME OVER";

    fill_rect(0, 0, SCREEN_W, SCREEN_H, COLOR_LCD);
    print_at((SCREEN_W - text_width(title, 3)) / 2, 90, 3, COLOR_INK, title);
    print_at((SCREEN_W - text_width("press RESET", 2)) / 2, 130, 2, COLOR_INK, "press RESET");
}

static void draw_screen(ScreenMode mode, const Pet *pet, const Assistant *ai, float time)
{
    Mood mood = pet_mood(pet);
    int x = bunny_x(pet, mood, time);

    for (int top = 0; top < SCREEN_H; top += STRIP_H) {
        gfx_begin_strip(top);
        if (mode == MODE_OVER) {
            draw_game_over(pet);
        } else {
            draw_status(ai);
            if (mode == MODE_STATS) {
                draw_stats(pet, ai);
            } else {
                draw_scene(pet, mood, x, BUNNY_Y, time);
            }
        }
        gfx_flush_strip();
    }
}

static long scene_key(const Pet *pet, float time)
{
    Mood mood = pet_mood(pet);
    long frame = mood == MOOD_HAPPY ? 0 : (long)(time * 5.0f);
    return frame * 1000L + (long)mood * 100L + (pet_is_fat(pet) ? 10 : 0) + (pet->clean < 50.0f ? 1 : 0);
}

static long screen_key(ScreenMode mode, const Pet *pet, float time)
{
    if (mode == MODE_OVER) {
        return 0;
    }
    if (mode == MODE_STATS) {
        return (long)time;
    }
    return scene_key(pet, time);
}

void screen_begin(void)
{
    lcd_init();
}

void screen_draw(const Pet *pet, const Assistant *ai, bool show_stats)
{
    float time = millis() / 1000.0f;
    ScreenMode mode = !pet_is_alive(pet) ? MODE_OVER : (show_stats ? MODE_STATS : MODE_SCENE);
    long key = screen_key(mode, pet, time);
    bool status_changed = ai->enabled != last_ai_enabled || strcmp(ai->message, last_message) != 0;

    if (mode == current_mode && key == last_key && !status_changed) {
        return;
    }
    current_mode = mode;
    last_key = key;
    last_ai_enabled = ai->enabled;
    strncpy(last_message, ai->message, sizeof(last_message) - 1);

    draw_screen(mode, pet, ai, time);
}
