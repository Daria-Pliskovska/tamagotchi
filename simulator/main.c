#include <Arduino.h>
#include <avr/interrupt.h>
#include "game.h"
#include "screen.h"

#define GAME_SPEED 2.0f
#define FRAME_MS 150
#define STATS_FRAME_MS 1000

enum {
    BTN_A,
    BTN_B,
    BTN_X,
    BTN_Y,
    BTN_STATS,
    BTN_AI,
    BTN_RESET,
    BTN_COUNT
};

static const int BUTTON_PINS[BTN_COUNT] = {2, 3, 4, 5, 6, 7, 12};

static Pet pet;
static Assistant ai;
static bool show_stats = false;
static volatile uint8_t pending_presses;
static unsigned long last_update_ms;
static unsigned long last_draw_ms;

static void new_game(void)
{
    pet_init(&pet);
    assistant_init(&ai);
}

static void handle_button(int button)
{
    switch (button) {
    case BTN_A:
        assistant_do(&ai, &pet, ACTION_FEED, "You");
        break;
    case BTN_B:
        assistant_do(&ai, &pet, ACTION_CHANGE, "You");
        break;
    case BTN_X:
        assistant_do(&ai, &pet, ACTION_PLAY, "You");
        break;
    case BTN_Y:
        assistant_do(&ai, &pet, ACTION_SLEEP, "You");
        break;
    case BTN_STATS:
        show_stats = !show_stats;
        break;
    case BTN_AI:
        assistant_toggle(&ai);
        break;
    case BTN_RESET:
        new_game();
        break;
    default:
        break;
    }
}

ISR(TIMER2_COMPA_vect)
{
    static uint8_t was_down = 0;
    uint8_t down = 0;
    for (int i = 0; i < BTN_COUNT; i++) {
        if (digitalRead(BUTTON_PINS[i]) == LOW) {
            down |= 1 << i;
        }
    }
    pending_presses |= down & ~was_down;
    was_down = down;
}

static void start_button_timer(void)
{
    TCCR2A = 1 << WGM21;
    TCCR2B = 1 << CS22;
    OCR2A = 249;
    TIMSK2 = 1 << OCIE2A;
}

static void read_buttons(void)
{
    noInterrupts();
    uint8_t presses = pending_presses;
    pending_presses = 0;
    interrupts();

    for (int i = 0; i < BTN_COUNT; i++) {
        if (presses & (1 << i)) {
            handle_button(i);
        }
    }
}

void setup(void)
{
    for (int i = 0; i < BTN_COUNT; i++) {
        pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    }
    start_button_timer();
    screen_begin();
    new_game();
    last_update_ms = millis();
}

void loop(void)
{
    unsigned long now = millis();
    float minutes = (now - last_update_ms) / 1000.0f * GAME_SPEED;
    last_update_ms = now;

    pet_update(&pet, minutes);
    assistant_update(&ai, &pet);
    read_buttons();

    unsigned long frame = show_stats ? STATS_FRAME_MS : FRAME_MS;
    if (now - last_draw_ms >= frame) {
        last_draw_ms = now;
        screen_draw(&pet, &ai, show_stats);
    }
}
