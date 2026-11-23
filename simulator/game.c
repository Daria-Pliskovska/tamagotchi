#include "game.h"
#include <stdio.h>
#include <string.h>

#define STAT_MAX 100.0f
#define STAT_LOW 35.0f
#define STAT_WARN 45.0f
#define STAT_CRITICAL 15.0f

#define HUNGER_DECAY 0.5f
#define CLEAN_DECAY 0.35f
#define FUN_DECAY 0.6f
#define ENERGY_DECAY 0.3f
#define ENERGY_SLEEP_GAIN 1.5f
#define SLEEP_DECAY_FACTOR 0.5f
#define HEALTH_LOSS 0.5f
#define HEALTH_GAIN 0.2f

#define FEED_AMOUNT 40.0f
#define PLAY_FUN 30.0f
#define PLAY_ENERGY 10.0f
#define ANIMATION_MINUTES 8.0f

#define HUNGER_MAX 160.0f
#define RUN_AWAY_MINUTES 60.0f

#define AI_THRESHOLD 65.0f
#define AI_BUSY_MINUTES 10.0f
#define AI_OVERDUE_WEIGHT 0.5f
#define AI_SLEEP_FACTOR 0.6f
#define AI_MIN_PLAY_ENERGY 25.0f

static float clamp_range(float value, float max)
{
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static float clamp_stat(float value)
{
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > STAT_MAX) {
        return STAT_MAX;
    }
    return value;
}

void pet_init(Pet *pet)
{
    pet->hunger = 80.0f;
    pet->clean = STAT_MAX;
    pet->fun = 70.0f;
    pet->energy = 90.0f;
    pet->health = STAT_MAX;
    pet->sleeping = false;
    pet->clock = 0.0f;
    pet->last_action = ACTION_NONE;
    pet->action_clock = 0.0f;
    pet->tantrum = 0.0f;
    pet->ran_away = false;
}

void pet_update(Pet *pet, float minutes)
{
    if (!pet_is_alive(pet)) {
        return;
    }
    pet->clock += minutes;

    if (pet->sleeping) {
        pet->hunger -= HUNGER_DECAY * SLEEP_DECAY_FACTOR * minutes;
        pet->clean -= CLEAN_DECAY * SLEEP_DECAY_FACTOR * minutes;
        pet->energy += ENERGY_SLEEP_GAIN * minutes;
        if (pet->energy >= STAT_MAX || pet->hunger < STAT_CRITICAL) {
            pet->sleeping = false;
        }
    } else {
        pet->hunger -= HUNGER_DECAY * minutes;
        pet->clean -= CLEAN_DECAY * minutes;
        pet->fun -= FUN_DECAY * minutes;
        pet->energy -= ENERGY_DECAY * minutes;
    }

    pet->hunger = clamp_range(pet->hunger, HUNGER_MAX);
    pet->clean = clamp_stat(pet->clean);
    pet->fun = clamp_stat(pet->fun);
    pet->energy = clamp_stat(pet->energy);

    if (pet->hunger < STAT_CRITICAL || pet->clean < STAT_CRITICAL) {
        pet->health -= HEALTH_LOSS * minutes;
    } else {
        pet->health += HEALTH_GAIN * minutes;
    }
    pet->health = clamp_stat(pet->health);

    if (!pet->sleeping && pet->energy < STAT_CRITICAL) {
        pet->tantrum += minutes;
        if (pet->tantrum >= RUN_AWAY_MINUTES) {
            pet->ran_away = true;
        }
    } else {
        pet->tantrum = 0.0f;
    }
}

bool pet_is_alive(const Pet *pet)
{
    return pet->health > 0.0f && !pet->ran_away;
}

bool pet_is_fat(const Pet *pet)
{
    return pet->hunger > STAT_MAX;
}

float pet_escape_progress(const Pet *pet)
{
    return pet->tantrum / RUN_AWAY_MINUTES;
}

static bool pet_is_animating(const Pet *pet, Action action)
{
    return pet->last_action == action &&
           pet->clock - pet->action_clock < ANIMATION_MINUTES;
}

Mood pet_mood(const Pet *pet)
{
    if (pet->sleeping) {
        return MOOD_SLEEPING;
    }
    if (pet->tantrum > 0.0f) {
        return MOOD_TANTRUM;
    }
    if (pet_is_animating(pet, ACTION_FEED)) {
        return MOOD_EATING;
    }
    if (pet_is_animating(pet, ACTION_PLAY)) {
        return MOOD_PLAYING;
    }
    if (pet->hunger < STAT_WARN) {
        return MOOD_HUNGRY;
    }
    if (pet->energy < STAT_WARN) {
        return MOOD_SLEEPY;
    }
    if (pet->clean < STAT_LOW || pet->fun < STAT_LOW) {
        return MOOD_SAD;
    }
    return MOOD_HAPPY;
}

static bool pet_apply(Pet *pet, Action action)
{
    switch (action) {
    case ACTION_FEED:
        pet->sleeping = false;
        pet->hunger = clamp_range(pet->hunger + FEED_AMOUNT, HUNGER_MAX);
        break;
    case ACTION_CHANGE:
        pet->clean = STAT_MAX;
        break;
    case ACTION_PLAY:
        if (pet->sleeping) {
            return false;
        }
        pet->fun = clamp_stat(pet->fun + PLAY_FUN);
        pet->energy = clamp_stat(pet->energy - PLAY_ENERGY);
        break;
    case ACTION_SLEEP:
        if (pet->sleeping) {
            return false;
        }
        pet->sleeping = true;
        pet->tantrum = 0.0f;
        break;
    default:
        return false;
    }
    pet->last_action = action;
    pet->action_clock = pet->clock;
    return true;
}

const char *action_name(Action action)
{
    switch (action) {
    case ACTION_FEED:
        return "feeding";
    case ACTION_CHANGE:
        return "new diaper";
    case ACTION_PLAY:
        return "playing";
    case ACTION_SLEEP:
        return "bedtime";
    default:
        return "";
    }
}

void assistant_init(Assistant *ai)
{
    memset(ai, 0, sizeof(*ai));
    ai->interval[ACTION_FEED] = 120.0f;
    ai->interval[ACTION_CHANGE] = 150.0f;
    ai->interval[ACTION_PLAY] = 90.0f;
    ai->interval[ACTION_SLEEP] = 240.0f;
    ai->enabled = true;
    snprintf(ai->message, sizeof(ai->message), "AI: I'm watching");
}

void assistant_set_enabled(Assistant *ai, bool enabled)
{
    ai->enabled = enabled;
    snprintf(ai->message, sizeof(ai->message), enabled ? "AI: turned ON" : "AI: turned OFF");
}

void assistant_toggle(Assistant *ai)
{
    assistant_set_enabled(ai, !ai->enabled);
}

int assistant_minutes_left(const Assistant *ai, const Pet *pet, Action action)
{
    return (int)(ai->last_done[action] + ai->interval[action] - pet->clock);
}

static float assistant_urgency(const Assistant *ai, const Pet *pet, Action action)
{
    float need;

    switch (action) {
    case ACTION_FEED:
        need = STAT_MAX - pet->hunger;
        break;
    case ACTION_CHANGE:
        need = STAT_MAX - pet->clean;
        break;
    case ACTION_PLAY:
        if (pet->sleeping || pet->energy < AI_MIN_PLAY_ENERGY) {
            return 0.0f;
        }
        need = STAT_MAX - pet->fun;
        break;
    case ACTION_SLEEP:
        if (pet->sleeping) {
            return 0.0f;
        }
        need = STAT_MAX - pet->energy;
        break;
    default:
        return 0.0f;
    }

    int left = assistant_minutes_left(ai, pet, action);
    if (left < 0) {
        need += -left * AI_OVERDUE_WEIGHT;
    }
    if (pet->sleeping) {
        need *= AI_SLEEP_FACTOR;
    }
    return need;
}

void assistant_do(Assistant *ai, Pet *pet, Action action, const char *who)
{
    if (!pet_apply(pet, action)) {
        return;
    }
    ai->last_done[action] = pet->clock;
    ai->times_done[action]++;
    snprintf(ai->message, sizeof(ai->message), "%s: %s", who, action_name(action));
}

void assistant_update(Assistant *ai, Pet *pet)
{
    if (!ai->enabled || !pet_is_alive(pet) || pet->clock < ai->busy_until) {
        return;
    }

    Action best = ACTION_NONE;
    float best_score = AI_THRESHOLD;

    for (int i = 0; i < ACTION_COUNT; i++) {
        float score = assistant_urgency(ai, pet, (Action)i);
        if (score >= best_score) {
            best_score = score;
            best = (Action)i;
        }
    }

    if (best != ACTION_NONE) {
        assistant_do(ai, pet, best, "AI");
        ai->busy_until = pet->clock + AI_BUSY_MINUTES;
    }
}