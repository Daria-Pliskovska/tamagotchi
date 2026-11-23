#ifndef SCREEN_H
#define SCREEN_H

#include "game.h"

void screen_begin(void);
void screen_draw(const Pet *pet, const Assistant *ai, bool show_stats);

#endif
