#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "SMSlib.h"
#ifdef USEPSGLIB
#include "PSGlib.h"
#endif
#include "assets.h"
#include "game.h"
#include "random.h"
#include "util.h"

static char start_level[8] = "APOLIMO";

void show_splash(void)
{
  SMS_initSprites();
  SMS_copySpritestoSAT();
  SMS_setBackdropColor(0);

  SMS_mapROMBank(retcon_tiles_bin_bank);
  SMS_loadTiles(retcon_tiles_bin, 0, retcon_tiles_bin_size);
  SMS_mapROMBank(retcon_tilemap_bin_bank);
  SMS_loadTileMap(0, 0, retcon_tilemap_bin, retcon_tilemap_bin_size);
  SMS_mapROMBank(font_bin_bank);
  SMS_loadTiles(GLYPH(' '), 32, font_bin_size);

  cancelable_wait(1);
  SMS_mapROMBank(retcon_palettes_bin_bank);
  fade_in(retcon_palettes_bin, &retcon_palettes_bin[16], retcon_palettes_bin_bank);
  cancelable_wait(3);
  fade_out(retcon_palettes_bin, &retcon_palettes_bin[16], retcon_palettes_bin_bank);
  clear_screen(" ");
  cancelable_wait(2);
  SMS_mapROMBank(retcon_palettes_bin_bank);
  SMS_printatXY(5, 11, "Undeveloper_ presents");
  fade_in(retcon_palettes_bin, &retcon_palettes_bin[16], retcon_palettes_bin_bank);
  cancelable_wait(3);
  fade_out(retcon_palettes_bin, &retcon_palettes_bin[16], retcon_palettes_bin_bank);
  clear_screen(" ");
  cancelable_wait(2);
  SMS_printatXY(6, 11, "A Retcon production");
  fade_in(retcon_palettes_bin, &retcon_palettes_bin[16], retcon_palettes_bin_bank);
  cancelable_wait(3);
  fade_out(retcon_palettes_bin, &retcon_palettes_bin[16], retcon_palettes_bin_bank);
  cancelable_wait(2);
  // SMS_mapROMBank(splash_lower_tiles_bin_bank);
  // SMS_loadTiles(splash_lower_tiles_bin, 0, splash_lower_tiles_bin_size);
  // SMS_mapROMBank(splash_upper_tiles_bin_bank);
  // SMS_loadTiles(splash_upper_tiles_bin, 256, splash_upper_tiles_bin_size);
  // SMS_mapROMBank(splash_tilemap_bin_bank);
  // SMS_loadTileMap(0, 1, splash_tilemap_bin, splash_tilemap_bin_size);
  // SMS_setNextTileatLoc(0);
  // for (int i = 0; i < 32; i++)
  //   SMS_setTile(10 | TILE_USE_SPRITE_PALETTE);
  // SMS_setNextTileatXY(0, 23);
  // for (int i = 0; i < 32; i++)
  //   SMS_setTile(10 | TILE_USE_SPRITE_PALETTE);
  // SMS_mapROMBank(splash_palette_bin_bank);
  // fade_in(splash_palette_bin, NULL, 0);
  // cancelable_wait(5);
  // fade_out(splash_palette_bin, NULL, 0);
  // cancelable_wait(1);
}

void fade_to_blank(void)
{
  SMS_mapROMBank(background_palette_bin_bank);
  fade_out(background_palette_bin, NULL, 0);
  clear_screen(" ");
  SMS_waitForVBlank();
}

void play_music(void)
{
#ifdef USEPSGLIB
  PSGRestoreVolumes();
  PSGSetMusicVolumeAttenuation(9);
  SMS_mapROMBank(primates_psg_bank);
  PSGPlay(&primates_psg);
#endif
}

void stop_music(void)
{
#ifdef USEPSGLIB
  PSGSilenceChannels();
  PSGStop();
#endif
}

bool show_code_menu(void)
{
  unsigned int keys;
  int len = 0;

  for (int i = 0; i < 8; i++)
    start_level[i] = '\0';

  SMS_waitForVBlank();
  fade_to_blank();
  SMS_printatXY(6, 7, "* Enter level code *");
  SMS_printatXY(4, 11, "Restore a previous game");
  SMS_printatXY(6, 13, "using the level code");

  SMS_mapROMBank(background_palette_bin_bank);
  fade_in(background_palette_bin, NULL, 0);

  while (true)
  {
    SMS_waitForVBlank();
#ifdef USEPSGLIB
    PSGFrame();
#endif
    SMS_printatXY(12 + len, 17, start_level[len] ? &start_level[len] : "_");
    keys = SMS_getKeysPressed();
    if ((keys & PORT_A_KEY_1) && start_level[len] > 'A')
    {
      len++;
      if (len == 7)
      {
        if (!random_deserialize_seed(start_level))
        {
          SMS_printatXY(8, 19, "Code not valid!");
          wait(1);
          while (!(SMS_getKeysPressed() & PORT_A_KEY_1))
            SMS_waitForVBlank();
          return false;
        }
        return true;
      }
    }
    if (keys & (PORT_A_KEY_UP | PORT_A_KEY_DOWN))
    {
      start_level[len] += (keys & PORT_A_KEY_UP) ? 1 : -1;
      if (start_level[len] < 'A')
        start_level[len] = 'A' + 15;
      else if (start_level[len] > 'A' + 15)
        start_level[len] = 'A';
    }
  }
}

void show_instructions_menu(void)
{
  unsigned int keys;

  SMS_waitForVBlank();
  fade_to_blank();
  SMS_printatXY(6, 7, "* Instructions *");
  SMS_printatXY(1, 11, "Guide the lander module safely");
  SMS_printatXY(6, 12, "onto the landing pad");
  SMS_printatXY(2, 14, "Left / right = thrust angle");
  SMS_printatXY(1, 15, "Action button = thrust");
  SMS_printatXY(5, 17, "Don't land too fast!!");

  SMS_mapROMBank(background_palette_bin_bank);
  fade_in(background_palette_bin, NULL, 0);

  while (true)
  {
    SMS_waitForVBlank();
#ifdef USEPSGLIB
    PSGFrame();
#endif
    keys = SMS_getKeysPressed();
    if (keys & PORT_A_KEY_1)
      return;
  }
}

void show_main_menu(void)
{
  int menu_option;
  unsigned int keys;

  while (true)
  {
    menu_option = 2;

    SMS_mapROMBank(font_bin_bank);
    SMS_loadTiles(GLYPH(' '), 32, font_bin_size);
    clear_screen(" ");
    SMS_printatXY(1, 7, "L A N D   O N   M Y   B A S E ");
    SMS_printatXY(1, 9, "(and tell me that you love me)");
    SMS_printatXY(8, 13, "Instructions");
    SMS_printatXY(8, 15, "Enter level code");
    SMS_printatXY(8, 17, "Start new game");

    SMS_mapROMBank(background_palette_bin_bank);
    fade_in(background_palette_bin, NULL, 0);

    SMS_printatXY(6, 13 + menu_option * 2, ">");

    while (true)
    {
      SMS_waitForVBlank();
#ifdef USEPSGLIB
      PSGFrame();
#endif
      keys = SMS_getKeysPressed();
      if (keys & (PORT_A_KEY_1 | PORT_A_KEY_2))
      {
        if (menu_option == 2)
        {
          return;
        }
        else if (menu_option == 1)
        {
          if (show_code_menu())
            return;
          break;
        }
        else
        {
          show_instructions_menu();
          break;
        }
      }
      if (keys & (PORT_A_KEY_UP | PORT_A_KEY_DOWN))
      {
        SMS_printatXY(6, 13 + menu_option * 2, " ");
        menu_option = (menu_option + (keys & PORT_A_KEY_DOWN ? 1 : 2)) % 3;
        SMS_printatXY(6, 13 + menu_option * 2, ">");
      }
    }
  }
}

void main(void)
{
  // show_splash();
  while (true)
  {
    // play_music();
    show_main_menu();
    stop_music();
    fade_to_blank();
    clear_screen(BLANK_TILE);
    game_loop(start_level);
  }
}
