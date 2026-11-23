#include "raylib.h"
#include "game.h"
#include "draw.h"

#define GAME_SPEED 2.0f

int main(void)
{
    Pet pet;
    Assistant ai;
    bool show_stats = false;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_W, SCREEN_H, "Tamagotchi AI");
    SetTargetFPS(60);

    RenderTexture2D canvas = LoadRenderTexture(SCREEN_W, SCREEN_H);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);
    draw_load();

    pet_init(&pet);
    assistant_init(&ai);

    while (!WindowShouldClose()) {
        pet_update(&pet, GetFrameTime() * GAME_SPEED);
        assistant_update(&ai, &pet);
        update_mouse_scale();

        switch (read_button()) {
        case BUTTON_A:
            assistant_do(&ai, &pet, ACTION_FEED, "You");
            break;
        case BUTTON_B:
            assistant_do(&ai, &pet, ACTION_CHANGE, "You");
            break;
        case BUTTON_X:
            assistant_do(&ai, &pet, ACTION_PLAY, "You");
            break;
        case BUTTON_Y:
            assistant_do(&ai, &pet, ACTION_SLEEP, "You");
            break;
        case BUTTON_STATS:
            show_stats = !show_stats;
            break;
        case BUTTON_AI:
            assistant_toggle(&ai);
            break;
        case BUTTON_RESTART:
            pet_init(&pet);
            assistant_init(&ai);
            break;
        default:
            break;
        }

        if (IsKeyPressed(KEY_F11)) {
            ToggleBorderlessWindowed();
        }

        BeginTextureMode(canvas);
        draw_frame(&pet, &ai, show_stats);
        EndTextureMode();
        present_frame(canvas);
    }

    draw_unload();
    UnloadRenderTexture(canvas);
    CloseWindow();
    return 0;
}