#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

typedef enum {
    ACTION_FEED,
    ACTION_CHANGE,
    ACTION_PLAY,
    ACTION_SLEEP,
    ACTION_COUNT,
    ACTION_NONE
} Action;

typedef enum {
    MOOD_HAPPY,
    MOOD_EATING,
    MOOD_PLAYING,
    MOOD_SLEEPING,
    MOOD_TANTRUM,
    MOOD_HUNGRY,
    MOOD_SLEEPY,
    MOOD_SAD
} Mood;

typedef struct {
    float hunger;
    float clean;
    float fun;
    float energy;
    float health;
    bool sleeping;
    float clock;
    Action last_action;
    float action_clock;
    float tantrum;
    bool ran_away;
} Pet;

typedef struct {
    float last_done[ACTION_COUNT];
    int times_done[ACTION_COUNT];
    float interval[ACTION_COUNT];
    float busy_until;
    bool enabled;
    char message[40];
} Assistant;

void pet_init(Pet *pet);
void pet_update(Pet *pet, float minutes);
bool pet_is_alive(const Pet *pet);
Mood pet_mood(const Pet *pet);
bool pet_is_fat(const Pet *pet);
float pet_escape_progress(const Pet *pet);

void assistant_init(Assistant *ai);
void assistant_update(Assistant *ai, Pet *pet);
void assistant_set_enabled(Assistant *ai, bool enabled);
void assistant_toggle(Assistant *ai);
void assistant_do(Assistant *ai, Pet *pet, Action action, const char *who);
int assistant_minutes_left(const Assistant *ai, const Pet *pet, Action action);
const char *action_name(Action action);

#endif