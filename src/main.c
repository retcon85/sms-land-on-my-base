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

static game_config_t game_config;
static bool config_loaded = false;

#pragma save
#pragma disable_warning 85
static void splash_bars(uint8_t freq, uint16_t repetitions)
{
  // freq will be in a
  // repetitions will be in de

  // init
  __asm__("ld l, a        ; save freq");
  __asm__("ld h, #2       ; first color (2)");
  __asm__("ex af, af';    ; use shadow a register");
  __asm__("ld a, #0x9f    ; ch1 volume (0)");
  __asm__("out(#0x7f), a  ; ch1 volume (0)");
  __asm__("ex af, af';    ; switch back to normal register");
  __asm__("ld a, #0x80    ; ch1 'AM' tone");
  __asm__("out(#0x7f), a  ; ch1 'AM' tone");
  __asm__("xor a          ; ch1 'AM' tone");
  __asm__("out(#0x7f), a  ; ch1 'AM' tone");

  // backdrop pattern
  __asm__("rpt:");
  __asm__("ld a, h        ; set backdrop color");
  __asm__("out(#0xbf), a  ; set backdrop color");
  __asm__("ld a, #0x87    ; set backdrop color");
  __asm__("out(#0xbf), a  ; set backdrop color");
  __asm__("ld b, l        ; restore freq");
  __asm__("wait:           ; wait");
  __asm__("djnz wait       ; wait");

  // sound
  __asm__("ex af, af'     ; use shadow register for volume");
  __asm__("cpl            ; complement volume value");
  __asm__("or #0x90       ; prepare for PSG volume register");
  __asm__("and #0x9f      ; prepare for PSG volume register");
  __asm__("out (#0x7f), a ; write volume to PSG");
  __asm__("ex af, af'     ; switch back to normal register");

  // switch backdrop color
  __asm__("ld a, h        ; load current color from h");
  __asm__("xor #0x01        ; flip bit 0");
  __asm__("ld h, a        ; store color to h");

  // repeat
  __asm__("dec e          ; decrement counter lsb");
  __asm__("jr nz, rpt     ; repeat if not zero");
  __asm__("xor a          ; test counter msb");
  __asm__("or d           ; test counter msb");
  __asm__("jr z, quit     ; if zero, quit");
  __asm__("dec d          ; decrement counter msb");
  __asm__("jr rpt         ; repeat");

  // quit
  __asm__("quit:");
  __asm__("ld a, #0x9f      ; silence audio channel");
  __asm__("out (#0x7f), a ; silence audio channel");
}
#pragma restore

void show_splash(void)
{
  // set up tile and palette for splash
  SMS_displayOff();
  SMS_initSprites();
  SMS_copySpritestoSAT();
  SMS_setBackdropColor(0);
  SMS_mapROMBank(retcon_tiles_bin_bank);
  SMS_loadTiles(retcon_tiles_bin, 0, retcon_tiles_bin_size);
  SMS_mapROMBank(font_bin_bank);
  SMS_loadTiles(GLYPH(' '), 32, font_bin_size);
  SMS_loadBGPalette(retcon_palettes_bin);
  SMS_loadSpritePalette(&retcon_palettes_bin[16]);
  SMS_mapROMBank(retcon_tilemap_bin_bank);
  SMS_loadTileMap(0, 0, retcon_tilemap_bin, retcon_tilemap_bin_size);
  SMS_mapROMBank(retcon_palettes_bin_bank);

  // do speccy jazz
  const char jazz_str[] = "AllWorkAndNoPlayMakesJackADullBoyAllWorkAndNoPlayMakesJackADullBoy";
  SMS_setSpritePaletteColor(2, 0x3c); // red
  SMS_setSpritePaletteColor(3, 0x03); // cyan
  __asm__("di");
  splash_bars(146, 3233);
  SMS_setSpritePaletteColor(2, 0x30); // blue
  SMS_setSpritePaletteColor(3, 0x0f); // yellow
  __asm__("di");
  for (register uint8_t i = 0; i < sizeof(jazz_str) - 1; i++)
  {
    splash_bars((jazz_str[i] & 0x01) ? 19 : 52, 2);
    splash_bars((jazz_str[i] & 0x02) ? 19 : 52, 2);
    splash_bars((jazz_str[i] & 0x04) ? 19 : 52, 2);
    splash_bars((jazz_str[i] & 0x08) ? 19 : 52, 2);
    splash_bars((jazz_str[i] & 0x10) ? 19 : 52, 2);
    splash_bars((jazz_str[i] & 0x20) ? 19 : 52, 2);
    splash_bars((jazz_str[i] & 0x40) ? 19 : 52, 2);
    splash_bars((jazz_str[i] & 0x80) ? 19 : 52, 2);
  }
  __asm__("ei");

  // display retcon logo for 3 seconds
  SMS_setBackdropColor(0);
  SMS_setSpritePaletteColor(3, 0);
  SMS_setSpritePaletteColor(4, 0);
  SMS_displayOn();
  cancelable_wait(2);

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
    game_config.level_code[i] = '\0';

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
    SMS_printatXY(12 + len, 17, game_config.level_code[len] ? &game_config.level_code[len] : "_");
    keys = SMS_getKeysPressed();
    if ((keys & PORT_A_KEY_1) && game_config.level_code[len] >= 'A')
    {
      len++;
      if (len == 7)
      {
        if (!random_deserialize_seed(game_config.level_code))
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
      game_config.level_code[len] += (keys & PORT_A_KEY_UP) ? 1 : -1;
      if (game_config.level_code[len] < 'A')
        game_config.level_code[len] = 'A' + 15;
      else if (game_config.level_code[len] > 'A' + 15)
        game_config.level_code[len] = 'A';
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

typedef enum
{
  INSTRUCTIONS,
  LEVEL_CODE,
  DIFFICULTY,
  INVERT_CONTROLS,
  NEW_GAME,
  CONTINUE,
} main_menu_options_t;

void show_main_menu(void)
{
  main_menu_options_t menu_option;
  unsigned int keys;
  int last_menu_item = config_loaded ? CONTINUE : NEW_GAME;

  const char difficulty_levels[][10] = {
      "Pointless",
      "Normal   ",
      "Crushing ",
  };

  while (true)
  {
    menu_option = last_menu_item;

    SMS_mapROMBank(font_bin_bank);
    SMS_loadTiles(GLYPH(' '), 32, font_bin_size);
    clear_screen(" ");
    int i = 5;
    int menu_i;
    SMS_printatXY(1, i, "L A N D   O N   M Y   B A S E ");
    i += 2;
    SMS_printatXY(1, i, "(and tell me that you love me)");
    i += 3;
    menu_i = i;
    SMS_printatXY(8, i, "Instructions");
    i += 2;
    SMS_printatXY(8, i, "Enter level code");
    i += 2;
    SMS_printatXY(8, i, "Difficulty - ");
    i += 2;
    SMS_printatXY(8, i, "Invert controls - ");
    i += 2;
    SMS_printatXY(8, i, "Start new game");
    i += 2;
    if (last_menu_item == CONTINUE)
      SMS_printatXY(8, i, "Continue");

    SMS_printatXY(6, menu_i + menu_option * 2, ">");

    SMS_mapROMBank(background_palette_bin_bank);
    fade_in(background_palette_bin, NULL, 0);

    while (true)
    {
      SMS_waitForVBlank();
#ifdef USEPSGLIB
      PSGFrame();
#endif
      SMS_printatXY(26, menu_i + INVERT_CONTROLS * 2, game_config.invert_controls ? "YES" : "NO ");
      SMS_printatXY(21, menu_i + DIFFICULTY * 2, difficulty_levels[game_config.difficulty]);
      keys = SMS_getKeysPressed();
      if (keys & (PORT_A_KEY_1 | PORT_A_KEY_2))
      {
        if (menu_option == CONTINUE)
        {
          return;
        }
        if (menu_option == NEW_GAME)
        {
          for (int i = 0; i < sizeof(game_config.level_code); i++)
            game_config.level_code[i] = DEFAULT_START_LEVEL[i];
          return;
        }
        else if (menu_option == LEVEL_CODE)
        {
          if (show_code_menu())
            return;
          break;
        }
        else if (menu_option == INSTRUCTIONS)
        {
          show_instructions_menu();
          break;
        }
        else if (menu_option == DIFFICULTY)
        {
          game_config.difficulty = (game_config.difficulty + 1) % 3;
        }
        else if (menu_option == INVERT_CONTROLS)
        {
          game_config.invert_controls = !game_config.invert_controls;
        }
      }
      if (keys & (PORT_A_KEY_UP | PORT_A_KEY_DOWN))
      {
        SMS_printatXY(6, menu_i + menu_option * 2, " ");
        menu_option = (menu_option + (keys & PORT_A_KEY_DOWN ? 1 : last_menu_item)) % (last_menu_item + 1);
        SMS_printatXY(6, menu_i + menu_option * 2, ">");
      }
    }
  }
}

void main(void)
{
  show_splash();
  while (true)
  {
    // play_music();
    config_loaded = game_load_config(&game_config);
    show_main_menu();
    stop_music();
    fade_to_blank();
    clear_screen(BLANK_TILE);
    game_run(&game_config);
  }
}
