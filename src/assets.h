#include "assets.generated.h"
#include <stdint.h>

extern const uint8_t lander_bg_palette[];
extern const uint8_t lander_sprite_palette[];
extern const uint8_t lander_tiles[];

#define CHR(c) (c - 32)
#define GLYPH(c) (font_bin + 32 * CHR(c))

#define BLANK_TILE 0
