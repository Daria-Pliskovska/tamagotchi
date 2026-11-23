#include "draw.h"
#include "raylib.h"
#include <math.h>

#define PX 8
#define ICON_PX 3
#define SMALL_PX 4

#define BUNNY_W 16
#define BUNNY_H 20
#define ICON_SIZE 8
#define BAR_CELLS 5
#define BAR_EXTRA_CELLS 3

#define LCD_X 300
#define LCD_Y 95
#define LCD_W 300
#define LCD_H 270

#define PAD_X 745
#define PAD_Y 230
#define PAD_STEP 55
#define BTN_R 26

#define STATS_X 305
#define STATS_Y 392
#define STATS_W 90
#define STATS_H 28

#define AI_BTN_X 405
#define AI_BTN_Y 392
#define AI_BTN_W 90
#define AI_BTN_H 28

#define RESET_X 505
#define RESET_Y 392
#define RESET_W 90
#define RESET_H 28

#define COLOR_BG         (Color){255, 245, 235, 255}
#define COLOR_BODY       (Color){240, 120, 170, 255}
#define COLOR_BODY_EDGE  (Color){200, 80, 130, 255}
#define COLOR_BEZEL      (Color){90, 90, 110, 255}
#define COLOR_LCD        (Color){200, 235, 215, 255}
#define COLOR_INK        (Color){40, 70, 60, 255}
#define COLOR_OUTLINE    (Color){40, 70, 140, 255}
#define COLOR_FUR        (Color){130, 190, 245, 255}
#define COLOR_PINK       (Color){245, 160, 190, 255}
#define COLOR_EYE        (Color){20, 20, 30, 255}
#define COLOR_TEAR       (Color){80, 160, 255, 255}
#define COLOR_DROOL      (Color){110, 180, 250, 255}
#define COLOR_POOP       (Color){140, 90, 50, 255}
#define COLOR_CARROT     (Color){240, 140, 40, 255}
#define COLOR_LEAF       (Color){60, 160, 60, 255}
#define COLOR_YELLOW     (Color){250, 210, 70, 255}
#define COLOR_HAT        (Color){230, 70, 120, 255}
#define COLOR_BTN_A      (Color){90, 190, 90, 255}
#define COLOR_BTN_B      (Color){230, 80, 80, 255}
#define COLOR_BTN_X      (Color){70, 130, 230, 255}
#define COLOR_BTN_Y      (Color){240, 190, 50, 255}
#define COLOR_ANGRY      (Color){220, 50, 50, 255}

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

static const char *ICONS[ACTION_COUNT][ICON_SIZE] = {
    { "...##...", "..####..", "..#..#..", ".######.", ".#....#.", ".#.##.#.", ".#....#.", ".######." },
    { "........", "########", "#......#", "#......#", ".#....#.", "..#..#..", "...##...", "........" },
    { "..####..", ".#....#.", "#.#..#.#", "#..##..#", "#..##..#", "#.#..#.#", ".#....#.", "..####.." },
    { "..###...", ".##.....", "##......", "##......", "##......", "##....#.", ".##..##.", "..####.." },
};

static const char *ACTION_LABELS[ACTION_COUNT] = {"Feed", "Change", "Play", "Sleep"};
static const char *BUTTON_LETTERS[ACTION_COUNT] = {"A", "B", "X", "Y"};

static Color button_color(int index)
{
    switch (index) {
    case ACTION_FEED:
        return COLOR_BTN_A;
    case ACTION_CHANGE:
        return COLOR_BTN_B;
    case ACTION_PLAY:
        return COLOR_BTN_X;
    default:
        return COLOR_BTN_Y;
    }
}

static Vector2 button_position(int index)
{
    switch (index) {
    case ACTION_FEED:
        return (Vector2){PAD_X, PAD_Y + PAD_STEP};
    case ACTION_CHANGE:
        return (Vector2){PAD_X + PAD_STEP, PAD_Y};
    case ACTION_PLAY:
        return (Vector2){PAD_X - PAD_STEP, PAD_Y};
    default:
        return (Vector2){PAD_X, PAD_Y - PAD_STEP};
    }
}

static Color palette_color(char c)
{
    switch (c) {
    case 'o':
        return COLOR_OUTLINE;
    case 'b':
        return COLOR_FUR;
    case 'p':
        return COLOR_PINK;
    case 'e':
        return COLOR_EYE;
    case 'w':
        return WHITE;
    case 'c':
        return COLOR_CARROT;
    case 'g':
        return COLOR_LEAF;
    default:
        return BLANK;
    }
}

static void draw_cell(int x, int y, int col, int row, int size, Color color)
{
    DrawRectangle(x + col * size, y + row * size, size, size, color);
}

static void draw_color_sprite(const char **rows, int height, int x, int y, int size)
{
    for (int row = 0; row < height; row++) {
        for (int col = 0; rows[row][col] != '\0'; col++) {
            Color color = palette_color(rows[row][col]);
            if (color.a != 0) {
                draw_cell(x, y, col, row, size, color);
            }
        }
    }
}

static void draw_mono_sprite(const char **rows, int height, int x, int y, int size, Color color)
{
    for (int row = 0; row < height; row++) {
        for (int col = 0; rows[row][col] != '\0'; col++) {
            if (rows[row][col] == '#') {
                draw_cell(x, y, col, row, size, color);
            }
        }
    }
}

static void set_eyes(int x, int y, Color top, Color bottom)
{
    int eye_cols[] = {3, 4, 11, 12};
    for (int i = 0; i < 4; i++) {
        draw_cell(x, y, eye_cols[i], 9, PX, top);
        draw_cell(x, y, eye_cols[i], 10, PX, bottom);
    }
}

static void draw_open_mouth(int x, int y)
{
    draw_cell(x, y, 7, 11, PX, COLOR_OUTLINE);
    draw_cell(x, y, 8, 11, PX, COLOR_OUTLINE);
    draw_cell(x, y, 7, 12, PX, COLOR_OUTLINE);
    draw_cell(x, y, 8, 12, PX, COLOR_OUTLINE);
}

static void draw_drool(int x, int y, float time)
{
    int length = 2 + (int)(time * 3) % 3;
    int drool_x = x + 8 * PX + 2;
    int drool_y = y + 12 * PX;
    DrawRectangle(drool_x, drool_y, SMALL_PX, SMALL_PX * length, COLOR_DROOL);
    DrawCircle(drool_x + SMALL_PX / 2, drool_y + SMALL_PX * length, 4, COLOR_DROOL);
}

static void draw_sleeping(int x, int y, float time)
{
    set_eyes(x, y, COLOR_FUR, COLOR_OUTLINE);
    draw_drool(x, y, time);
    int shift = (int)(time * 2) % 3 * 3;
    DrawText("z", x + BUNNY_W * PX, y + 40 - shift, 20, COLOR_INK);
    DrawText("Z", x + BUNNY_W * PX + 16, y + 15 - shift, 26, COLOR_INK);
}

static void draw_eating(int x, int y, float time)
{
    if ((int)(time * 5) % 2 == 0) {
        draw_open_mouth(x, y);
    }
    draw_color_sprite(CARROT_HELD, ICON_SIZE, x + 8 * PX + 4, y + 11 * PX, SMALL_PX);
}

static void draw_party_hat(int x, int y)
{
    int center = x + 8 * PX;
    int base = y + 6 * PX;
    int rows = 10;
    int top = base - rows * SMALL_PX;

    for (int k = 0; k < rows; k++) {
        Color stripe = (k / 2) % 2 == 0 ? COLOR_HAT : COLOR_YELLOW;
        DrawRectangle(center - k * 2 - 2, top + k * SMALL_PX, k * 4 + 4, SMALL_PX, stripe);
    }
    DrawRectangle(center - 4, top - 8, 8, 8, WHITE);
}

static void draw_confetti(float time)
{
    Color colors[] = {COLOR_HAT, COLOR_YELLOW, COLOR_BTN_X, COLOR_BTN_A};
    for (int i = 0; i < 14; i++) {
        int cx = LCD_X + 10 + (i * 67) % (LCD_W - 20);
        int cy = LCD_Y + 40 + (int)fmodf(time * 50.0f + i * 29.0f, LCD_H - 80);
        DrawRectangle(cx, cy, 6, 6, colors[i % 4]);
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

static void draw_cloud_circle(int cx, int cy, int r)
{
    DrawCircle(cx, cy, r + 2, COLOR_INK);
}

static void draw_cloud_fill(int cx, int cy, int r)
{
    DrawCircle(cx, cy, r, WHITE);
}

static void draw_thought_cloud(int x, int y)
{
    int cx = LCD_X + LCD_W - 52;
    int cy = LCD_Y + 58;
    int parts[4][3] = {{-16, 4, 18}, {14, 4, 18}, {0, -10, 20}, {0, 12, 16}};

    for (int i = 0; i < 4; i++) {
        draw_cloud_circle(cx + parts[i][0], cy + parts[i][1], parts[i][2]);
    }
    for (int i = 0; i < 4; i++) {
        draw_cloud_fill(cx + parts[i][0], cy + parts[i][1], parts[i][2]);
    }

    DrawCircle(x + BUNNY_W * PX - 2, y + 7 * PX, 5, COLOR_INK);
    DrawCircle(x + BUNNY_W * PX - 2, y + 7 * PX, 3, WHITE);
    DrawCircle(x + BUNNY_W * PX + 12, y + 5 * PX, 7, COLOR_INK);
    DrawCircle(x + BUNNY_W * PX + 12, y + 5 * PX, 5, WHITE);

    draw_color_sprite(CARROT_UP, ICON_SIZE, cx - 16, cy - 16, SMALL_PX);
}

static void draw_hungry(int x, int y, float time)
{
    draw_open_mouth(x, y);
    draw_drool(x, y, time);
    draw_thought_cloud(x, y);
}

static void draw_sleepy(int x, int y, float time)
{
    bool yawning = fmodf(time, 3.0f) < 1.0f;

    if (yawning) {
        set_eyes(x, y, COLOR_FUR, COLOR_OUTLINE);
        for (int col = 6; col <= 9; col++) {
            for (int row = 11; row <= 13; row++) {
                draw_cell(x, y, col, row, PX, COLOR_OUTLINE);
            }
        }
        draw_cell(x, y, 7, 12, PX, COLOR_PINK);
        draw_cell(x, y, 8, 12, PX, COLOR_PINK);
        DrawText("yawn", x + BUNNY_W * PX + 6, y + 50, 16, COLOR_INK);
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
        DrawText("NO!!", x - 30, y + 10, 22, COLOR_ANGRY);
    }
}

static void draw_bunny(const Pet *pet, float time)
{
    int x = LCD_X + (LCD_W - BUNNY_W * PX) / 2;
    int y = LCD_Y + 50;

    if (pet_mood(pet) == MOOD_TANTRUM) {
        x += (int)(pet_escape_progress(pet) * 140.0f);
        x += (int)(sinf(time * 40.0f) * 4.0f);
    }

    draw_color_sprite(pet_is_fat(pet) ? BUNNY_FAT : BUNNY, BUNNY_H, x, y, PX);

    switch (pet_mood(pet)) {
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

static void draw_bar(int x, int y, const char *name, float value)
{
    DrawText(name, x, y, 16, COLOR_INK);
    for (int i = 0; i < BAR_CELLS; i++) {
        Rectangle cell = {x + 90 + i * 22, y, 18, 14};
        if (value > i * 20) {
            DrawRectangleRec(cell, COLOR_INK);
        } else {
            DrawRectangleLinesEx(cell, 2, COLOR_INK);
        }
    }
    for (int i = BAR_CELLS; i < BAR_CELLS + BAR_EXTRA_CELLS; i++) {
        if (value > i * 20) {
            DrawRectangle(x + 90 + i * 22, y, 18, 14, COLOR_CARROT);
        }
    }
}

static void draw_stats(const Pet *pet, const Assistant *ai)
{
    int x = LCD_X + 20;
    int y = LCD_Y + 15;

    draw_bar(x, y, "HUNGER", pet->hunger);
    draw_bar(x, y + 25, "DIAPER", pet->clean);
    draw_bar(x, y + 50, "FUN", pet->fun);
    draw_bar(x, y + 75, "ENERGY", pet->energy);
    draw_bar(x, y + 100, "HEALTH", pet->health);

    DrawText(TextFormat("AI memory (%s):", ai->enabled ? "ON" : "OFF"), x, y + 135, 16, COLOR_INK);
    for (int i = 0; i < ACTION_SLEEP; i++) {
        const char *line = TextFormat("%s %dx, next %dm", action_name((Action)i),
                                      ai->times_done[i], assistant_minutes_left(ai, pet, (Action)i));
        DrawText(line, x, y + 158 + i * 20, 16, COLOR_INK);
    }
}

static Texture2D background;

void draw_load(void)
{
    background = LoadTexture("background.png");
}

void draw_unload(void)
{
    UnloadTexture(background);
}

static void draw_background(void)
{
    if (background.id == 0) {
        return;
    }
    Rectangle source = {0, 0, (float)background.width, (float)background.height};
    Rectangle dest = {LCD_X, LCD_Y, LCD_W, LCD_H};
    DrawTexturePro(background, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
}

static void draw_main_screen(const Pet *pet, const Assistant *ai, float time)
{
    draw_background();

    BeginScissorMode(LCD_X, LCD_Y, LCD_W, LCD_H);
    draw_bunny(pet, time);
    EndScissorMode();

    if (pet->clean < 50.0f) {
        draw_mono_sprite(POOP, ICON_SIZE, LCD_X + 15, LCD_Y + LCD_H - 70, SMALL_PX, COLOR_POOP);
    }

    DrawRectangle(LCD_X, LCD_Y + LCD_H - 28, LCD_W, 28, Fade(WHITE, 0.7f));
    DrawText(ai->message, LCD_X + 10, LCD_Y + LCD_H - 24, 16, COLOR_INK);
}

static void draw_body_shape(int offset, Color color)
{
    DrawCircle(170, 406 + offset, 110, color);
    DrawCircle(730, 406 + offset, 110, color);
    DrawRectangleRounded((Rectangle){40, 40 + offset, 820, 400}, 0.35f, 16, color);
}

static void draw_body(void)
{
    Rectangle bezel = {LCD_X - 15, LCD_Y - 15, LCD_W + 30, LCD_H + 30};

    draw_body_shape(6, COLOR_BODY_EDGE);
    draw_body_shape(0, COLOR_BODY);
    DrawText("TAMAGOTCHI AI", SCREEN_W / 2 - MeasureText("TAMAGOTCHI AI", 20) / 2, 52, 20, WHITE);
    DrawRectangleRounded(bezel, 0.08f, 8, COLOR_BEZEL);
    DrawRectangle(LCD_X, LCD_Y, LCD_W, LCD_H, COLOR_LCD);
}

static void draw_legend(void)
{
    for (int i = 0; i < ACTION_COUNT; i++) {
        int y = 130 + i * 58;
        DrawCircle(95, y + 12, 16, button_color(i));
        DrawText(BUTTON_LETTERS[i], 95 - MeasureText(BUTTON_LETTERS[i], 20) / 2, y + 2, 20, WHITE);
        draw_mono_sprite(ICONS[i], ICON_SIZE, 125, y, ICON_PX, WHITE);
        DrawText(ACTION_LABELS[i], 162, y + 2, 22, WHITE);
    }
}

static void draw_buttons(const Assistant *ai)
{
    for (int i = 0; i < ACTION_COUNT; i++) {
        Vector2 p = button_position(i);
        DrawCircle((int)p.x, (int)p.y + 4, BTN_R, COLOR_BODY_EDGE);
        DrawCircle((int)p.x, (int)p.y, BTN_R, button_color(i));
        DrawText(BUTTON_LETTERS[i], (int)p.x - MeasureText(BUTTON_LETTERS[i], 24) / 2,
                 (int)p.y - 12, 24, WHITE);
    }

    Rectangle stats = {STATS_X, STATS_Y, STATS_W, STATS_H};
    DrawRectangleRounded(stats, 0.6f, 8, COLOR_BEZEL);
    DrawText("STATS", STATS_X + STATS_W / 2 - MeasureText("STATS", 16) / 2, STATS_Y + 6, 16, WHITE);

    Rectangle ai_btn = {AI_BTN_X, AI_BTN_Y, AI_BTN_W, AI_BTN_H};
    const char *ai_label = ai->enabled ? "AI: ON" : "AI: OFF";
    DrawRectangleRounded(ai_btn, 0.6f, 8, ai->enabled ? COLOR_BTN_A : COLOR_BTN_B);
    DrawText(ai_label, AI_BTN_X + AI_BTN_W / 2 - MeasureText(ai_label, 16) / 2, AI_BTN_Y + 6, 16, WHITE);

    Rectangle reset = {RESET_X, RESET_Y, RESET_W, RESET_H};
    DrawRectangleRounded(reset, 0.6f, 8, COLOR_BEZEL);
    DrawText("RESET", RESET_X + RESET_W / 2 - MeasureText("RESET", 16) / 2, RESET_Y + 6, 16, WHITE);

    const char *hint = "ABXY - actions   S - stats   I - AI   R - restart";
    DrawText(hint, SCREEN_W / 2 - MeasureText(hint, 16) / 2, 492, 16, COLOR_BODY_EDGE);
}

void draw_frame(const Pet *pet, const Assistant *ai, bool show_stats)
{
    float time = (float)GetTime();

    ClearBackground(COLOR_BG);

    draw_body();

    if (!pet_is_alive(pet)) {
        const char *title = pet->ran_away ? "RAN AWAY!" : "GAME OVER";
        DrawText(title, LCD_X + LCD_W / 2 - MeasureText(title, 30) / 2, LCD_Y + 100, 30, COLOR_INK);
        DrawText("press RESET", LCD_X + LCD_W / 2 - MeasureText("press RESET", 20) / 2, LCD_Y + 140, 20, COLOR_INK);
    } else if (show_stats) {
        draw_stats(pet, ai);
    } else {
        draw_main_screen(pet, ai, time);
    }

    draw_legend();
    draw_buttons(ai);
}

static float screen_scale(void)
{
    float scale_x = (float)GetScreenWidth() / SCREEN_W;
    float scale_y = (float)GetScreenHeight() / SCREEN_H;
    return scale_x < scale_y ? scale_x : scale_y;
}

static Vector2 screen_offset(float scale)
{
    Vector2 offset;
    offset.x = (GetScreenWidth() - SCREEN_W * scale) / 2.0f;
    offset.y = (GetScreenHeight() - SCREEN_H * scale) / 2.0f;
    return offset;
}

void update_mouse_scale(void)
{
    float scale = screen_scale();
    Vector2 offset = screen_offset(scale);
    SetMouseOffset((int)-offset.x, (int)-offset.y);
    SetMouseScale(1.0f / scale, 1.0f / scale);
}

void present_frame(RenderTexture2D canvas)
{
    float scale = screen_scale();
    Vector2 offset = screen_offset(scale);
    Rectangle source = {0, 0, (float)canvas.texture.width, -(float)canvas.texture.height};
    Rectangle dest = {offset.x, offset.y, SCREEN_W * scale, SCREEN_H * scale};

    BeginDrawing();
    ClearBackground(COLOR_BG);
    DrawTexturePro(canvas.texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    EndDrawing();
}

Button read_button(void)
{
    if (IsKeyPressed(KEY_A)) {
        return BUTTON_A;
    }
    if (IsKeyPressed(KEY_B)) {
        return BUTTON_B;
    }
    if (IsKeyPressed(KEY_X)) {
        return BUTTON_X;
    }
    if (IsKeyPressed(KEY_Y)) {
        return BUTTON_Y;
    }
    if (IsKeyPressed(KEY_S)) {
        return BUTTON_STATS;
    }
    if (IsKeyPressed(KEY_I)) {
        return BUTTON_AI;
    }
    if (IsKeyPressed(KEY_R)) {
        return BUTTON_RESTART;
    }
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return BUTTON_NONE;
    }

    Vector2 mouse = GetMousePosition();
    Button action_buttons[ACTION_COUNT] = {BUTTON_A, BUTTON_B, BUTTON_X, BUTTON_Y};

    for (int i = 0; i < ACTION_COUNT; i++) {
        if (CheckCollisionPointCircle(mouse, button_position(i), BTN_R)) {
            return action_buttons[i];
        }
    }
    if (CheckCollisionPointRec(mouse, (Rectangle){STATS_X, STATS_Y, STATS_W, STATS_H})) {
        return BUTTON_STATS;
    }
    if (CheckCollisionPointRec(mouse, (Rectangle){AI_BTN_X, AI_BTN_Y, AI_BTN_W, AI_BTN_H})) {
        return BUTTON_AI;
    }
    if (CheckCollisionPointRec(mouse, (Rectangle){RESET_X, RESET_Y, RESET_W, RESET_H})) {
        return BUTTON_RESTART;
    }
    return BUTTON_NONE;
}