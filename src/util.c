#include "util.h"
#include <stdlib.h>
#include <stdint.h>

inline void clear_screen(const char *with)
{
  SMS_setNextTileatLoc(0);
  for (int i = 0; i < 896; i++)
    SMS_print(with);
}

const unsigned char color_reduction[] = {RGB(0, 0, 0), RGB(0, 0, 1), RGB(0, 1, 1), RGB(1, 1, 1),
                                         RGB(1, 1, 2), RGB(1, 2, 2), RGB(2, 2, 2),
                                         RGB(2, 2, 3), RGB(2, 3, 3), RGB(3, 3, 3)};

#define COLOR_SUBTRACT(c, r) (((((r) & 0x3) >= ((c) & 0x3)) ? 0 : (((c) & 0x3) - ((r) & 0x3))) | ((((r) & 0xC) >= ((c) & 0xC)) ? 0 : (((c) & 0xC) - ((r) & 0xC))) | ((((r) & 0x30) >= ((c) & 0x30)) ? 0 : (((c) & 0x30) - ((r) & 0x30))))

void _SMS_loadBGPaletteafterColorSubtraction(const void *palette, const unsigned char subtraction_color)
{
  unsigned char i;
  unsigned char j = 0;
  SMS_setNextBGColoratIndex(0);
  // precalculate the first four colors so that we tie up the CPU long enough to miss the bottom border
  unsigned char new_colors[16];
  for (i = 0; i < 16; j = i++)
  {
    new_colors[i] = COLOR_SUBTRACT(((unsigned char *)(palette))[i], subtraction_color);
    if (SMS_getVCount() >= 216)
      break;
  }
  for (i = 0; i <= j; i++)
    SMS_setColor(new_colors[i]);

  for (; i < 16; i++)
    SMS_setColor(COLOR_SUBTRACT(((unsigned char *)(palette))[i], subtraction_color));
}

void _SMS_loadSpritePaletteafterColorSubtraction(const void *palette, const unsigned char subtraction_color)
{
  unsigned char i;
  unsigned char j = 0;
  SMS_setNextSpriteColoratIndex(0);
  // precalculate the first four colors so that we tie up the CPU long enough to miss the bottom border
  unsigned char new_colors[16];
  for (i = 0; i < 16; j = i++)
  {
    new_colors[i] = COLOR_SUBTRACT(((unsigned char *)(palette))[i], subtraction_color);
    if (SMS_getVCount() >= 216)
      break;
  }
  for (i = 0; i <= j; i++)
    SMS_setColor(new_colors[i]);

  for (; i < 16; i++)
    SMS_setColor(COLOR_SUBTRACT(((unsigned char *)(palette))[i], subtraction_color));
}

void fade_in(const void *bg_palette, const void *sprite_palette, unsigned int sprite_palette_bank)
{
  SMS_displayOff();
  for (int i = sizeof(color_reduction) - 1; i >= 0; i--)
  {
    SMS_waitForVBlank();

    if (bg_palette != NULL)
      _SMS_loadBGPaletteafterColorSubtraction(bg_palette, color_reduction[i]);

    SMS_waitForVBlank();
    if (sprite_palette != NULL)
    {
      SMS_saveROMBank();
      SMS_mapROMBank(sprite_palette_bank);
      _SMS_loadSpritePaletteafterColorSubtraction(sprite_palette, color_reduction[i]);
      SMS_restoreROMBank();
    }
    if (i == sizeof(color_reduction) - 1)
      SMS_displayOn();
  }
}

void fade_out(const void *bg_palette, const void *sprite_palette, unsigned int sprite_palette_bank)
{
  for (int i = 0; i < sizeof(color_reduction); i++)
  {
    SMS_waitForVBlank();

    if (bg_palette != NULL)
      _SMS_loadBGPaletteafterColorSubtraction(bg_palette, color_reduction[i]);

    SMS_waitForVBlank();
    if (sprite_palette != NULL)
    {
      SMS_saveROMBank();
      SMS_mapROMBank(sprite_palette_bank);
      _SMS_loadSpritePaletteafterColorSubtraction(sprite_palette, color_reduction[i]);
      SMS_restoreROMBank();
    }
  }
  SMS_displayOff();
}

inline void cancelable_wait(int seconds)
{
  for (int i = 0; i < 60 * seconds; i++)
  {
    SMS_waitForVBlank();
    if (SMS_getKeysHeld())
      return;
  }
}

inline void wait(int seconds)
{
  for (int i = 0; i < 60 * seconds; i++)
    SMS_waitForVBlank();
}

static uint8_t flash_color = 0x00;

inline void flash_text(void)
{
  SMS_setSpritePaletteColor(1, flash_color);
  flash_color = ~flash_color;
}

inline void flash_text_hide(void)
{
  flash_color = 0x00;
  flash_text();
}

inline void print_dec1(unsigned int x, unsigned int y, int value, unsigned int pad)
{
  SMS_setTileatXY(x, y, value % 10 + '0');
}

inline void print_dec2(unsigned int x, unsigned int y, int value, unsigned int pad)
{
  SMS_setTileatXY(x, y, value >= 10 ? value / 10 + '0' : pad);
  print_dec1(x + 1, y, value % 10, '0');
}

inline void print_dec3(unsigned int x, unsigned int y, int value, unsigned int pad)
{
  SMS_setTileatXY(x, y, value >= 100 ? value / 100 + '0' : pad);
  print_dec2(x + 1, y, value % 100, '0');
}
