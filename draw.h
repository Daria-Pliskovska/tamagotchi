#ifndef DRAW_H
#define DRAW_H

#include <stdbool.h>
#include "game.h"
#include "raylib.h"

#define SCREEN_W 900
#define SCREEN_H 520

typedef enum {
    BUTTON_NONE,
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_STATS,
    BUTTON_AI,
    BUTTON_RESTART
} Button;

void draw_frame(const Pet *pet, const Assistant *ai, bool show_stats);
Button read_button(void);
void update_mouse_scale(void);
void draw_load(void);
void draw_unload(void);
void present_frame(RenderTexture2D canvas);

#endif