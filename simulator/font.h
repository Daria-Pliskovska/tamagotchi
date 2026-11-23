#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include <avr/pgmspace.h>

#define FONT_FIRST 32
#define FONT_CHARS 95
#define FONT_W 5
#define FONT_H 7

extern const uint8_t FONT[FONT_CHARS * FONT_W] PROGMEM;

#endif
