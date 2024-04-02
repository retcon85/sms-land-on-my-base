#include "game_view.h"
#include <stdbool.h>
#include "SMSlib.h"
#include "PSGlib.h"
#include "assets.h"
#include "util.h"

#define SHIP_SPRITES 0
#define FLAME_SPRITES (SHIP_SPRITES + (ship_tiles_bin_size / 32))
#define TICK_SPRITES (FLAME_SPRITES + (flames_tiles_bin_size / 32))
#define FUEL_SPRITES (TICK_SPRITES + (flames_tiles_bin_size / 32))
#define SPRITES_START 256

void game_view_init(game_model_t *model)
{
  // clear VRAM
  SMS_VRAMmemset(0, 0, 0x4000);
  SMS_mapROMBank(game_palette_bin_bank);
  SMS_loadBGPalette(game_palette_bin);
  SMS_loadSpritePalette(game_palette_bin);
  SMS_mapROMBank(surface_tiles_bin_bank);
  SMS_loadTiles(surface_tiles_bin, 1, surface_tiles_bin_size);
  SMS_mapROMBank(ship_tiles_bin_bank);
  SMS_loadTiles(ship_tiles_bin, SPRITES_START + SHIP_SPRITES, ship_tiles_bin_size);
  SMS_mapROMBank(flames_tiles_bin_bank);
  SMS_loadTiles(flames_tiles_bin, SPRITES_START + FLAME_SPRITES, flames_tiles_bin_size);
  SMS_mapROMBank(fuel_tiles_bin_bank);
  SMS_loadTiles(fuel_tiles_bin, SPRITES_START + FUEL_SPRITES, fuel_tiles_bin_size);
  SMS_mapROMBank(ticks_tiles_bin_bank);
  SMS_loadTiles(ticks_tiles_bin, SPRITES_START + TICK_SPRITES, ticks_tiles_bin_size);
  SMS_mapROMBank(font_bin_bank);
  SMS_loadTiles(GLYPH(' '), ' ', font_bin_size);

  for (int i = 0; i < LANDSCAPE_WIDTH; i++)
  {
    int j = 24 - model->landscape[i].view_y;
    if (i == model->pad_pos)
    {
      SMS_setTileatXY(i, j++, 3);
    }
    else if (i < LANDSCAPE_WIDTH - 1 && model->landscape[i + 1].view_y > model->landscape[i].view_y)
    {
      j--;
      SMS_setTileatXY(i, j++, 1);
      SMS_setTileatXY(i, j++, 8);
    }
    else if (i < LANDSCAPE_WIDTH - 1 && model->landscape[i + 1].view_y < model->landscape[i].view_y)
    {
      SMS_setTileatXY(i, j++, 1 | TILE_FLIPPED_X);
      SMS_setTileatXY(i, j++, 8 | TILE_FLIPPED_X);
    }
    else
    {
      SMS_setTileatXY(i, j++, 2);
      if (i % 3 == 0)
        SMS_setTileatXY(i, j++, 9 | (i % 2 == 0 ? TILE_FLIPPED_X : 0));
    }
    for (; j < 24; j++)
      SMS_setTileatXY(i, j, 10);
  }

  SMS_printatXY(0, 22, "Gravity   :                     ");
  print_dec1(15, 22, model->gravity, ' ');
  SMS_printatXY(0, 23, "Solar wind:                     ");
  // SMS_printatXY(20, 22, "Score: 12345");
  // SMS_printatXY(20, 23, "Level: 65536");
  if (model->wind_speed > 0)
  {
    SMS_printatXY(12, 23, "+");
    print_dec3(13, 23, model->wind_speed, ' ');
  }
  else
  {
    SMS_printatXY(12, 23, "-");
    print_dec3(13, 23, -model->wind_speed, ' ');
  }

  SMS_setSpritePaletteColor(10, 0x0e);

  SMS_configureTextRenderer(TILE_USE_SPRITE_PALETTE);
  SMS_printatXY(11, 9, "* PAUSED *");
  SMS_configureTextRenderer(0);
  flash_text_hide();

  SMS_displayOn();
}

typedef struct
{
  uint8_t preamble[6];
  uint8_t volume;
  uint8_t tone;
  uint8_t end;
} thrust_sfx_t;

static thrust_sfx_t thrust_sfx = {
    .preamble = {
        0xdf, // set tone 3 volume to silent
        0xcf, // set tone 3 tone to ....
        0x40, // ....0x002
        0xe7, // set noise tone to white noise, freq from tone 3
        0x01, // loop
        0x38, // wait
    },
    .volume = 0xff, // silent
    .tone = 0x38,   // wait
    .end = 0x00,
};

static void game_show_landed_screen(game_model_t *model)
{
#ifdef USEPSGLIB
  PSGSFXStop();
#endif
  // SMS_setBackdropColor(6);
  SMS_printatXY(10, 0, "* LANDED! *");
  for (int i = 0; i < 64; i++)
    SMS_waitForVBlank();
  SMS_printatXY(9, 2, "Code: ");
  SMS_printatXY(15, 2, model->level_code);
  for (int i = 0; i < 64; i++)
    SMS_waitForVBlank();
  SMS_configureTextRenderer(TILE_USE_SPRITE_PALETTE);
  SMS_printatXY(3, 5, "Press button for next level");
  SMS_configureTextRenderer(0);
  while (true)
  {
    for (int i = 0; i < 64; i++)
    {
      SMS_waitForVBlank();
      if (SMS_getKeysPressed() & (PORT_A_KEY_1 | PORT_A_KEY_2))
        return;
    }
    flash_text();
  }
}

static void game_show_crashed_screen(void)
{
#ifdef USEPSGLIB
  PSGSFXStop();
#endif
  // SMS_setBackdropColor(3);
  SMS_printatXY(10, 0, "* CRASHED! *");
  for (int i = 0; i < 64; i++)
    SMS_waitForVBlank();
  SMS_configureTextRenderer(TILE_USE_SPRITE_PALETTE);
  SMS_printatXY(6, 3, "Press button to retry");
  SMS_configureTextRenderer(0);
  while (true)
  {
    for (int i = 0; i < 64; i++)
    {
      SMS_waitForVBlank();
      if (SMS_getKeysPressed() & (PORT_A_KEY_1 | PORT_A_KEY_2))
        return;
    }
    flash_text();
  }
}

bool game_view_update(game_model_t *model)
{
#ifdef USEPSGLIB
  PSGSFXFrame();
#endif

  SMS_copySpritestoSAT();
  SMS_initSprites();

  if (model->landed)
  {
    game_show_landed_screen(model);
    return true;
  }

  if (model->collision)
  {
    game_show_crashed_screen();
    return true;
  }

  uint8_t thrust_level = (16 * model->ship.thrust) / MAX_THRUST;

  if (!model->ship.oob)
  {
    unsigned int x = model->ship.x / SCALE;
    unsigned int y = 192 - 8 - model->ship.y / SCALE;

    SMS_addSprite(x, y, SHIP_SPRITES);
    x += 12;
    y -= 12;
    SMS_addSprite(x, y, FUEL_SPRITES);
    if (model->ship.fuel >= 3 * UINT16_MAX / 8)
    {
      SMS_addSprite(x, y, FUEL_SPRITES + 2);
      x += 7;
      SMS_addSprite(x, y, FUEL_SPRITES + 1);
      x += 1;
      if (model->ship.fuel >= 7 * UINT16_MAX / 8)
        SMS_addSprite(x, y, FUEL_SPRITES + 2);
      else if (model->ship.fuel >= 6 * UINT16_MAX / 8)
        SMS_addSprite(x, y, FUEL_SPRITES + 3);
      else if (model->ship.fuel >= 5 * UINT16_MAX / 8)
        SMS_addSprite(x, y, FUEL_SPRITES + 4);
      else if (model->ship.fuel >= 4 * UINT16_MAX / 8)
        SMS_addSprite(x, y, FUEL_SPRITES + 5);
      x -= 20;
    }
    else
    {
      if (model->ship.fuel >= 2 * UINT16_MAX / 8)
        SMS_addSprite(x, y, FUEL_SPRITES + 3);
      else if (model->ship.fuel >= 1 * UINT16_MAX / 8)
        SMS_addSprite(x, y, FUEL_SPRITES + 4);
      else if (model->ship.fuel > 0)
        SMS_addSprite(x, y, FUEL_SPRITES + 5);
      x += 7;
      SMS_addSprite(x, y, FUEL_SPRITES + 1);
      x -= 19;
    }

    if (model->ship.fuel < 2 * UINT16_MAX / 8)
      SMS_setSpritePaletteColor(10, 0x03);
    else if (model->ship.fuel < 4 * UINT16_MAX / 8)
      SMS_setSpritePaletteColor(10, 0x07);
    else if (model->ship.fuel < 6 * UINT16_MAX / 8)
      SMS_setSpritePaletteColor(10, 0x0f);

    y += 20;
    SMS_addSprite(x, y, FLAME_SPRITES + (thrust_level / 4));
    y += (model->ship.angle_y / 32) - 6;
    x -= model->ship.angle_x / 32;
    SMS_addSprite(x, y, TICK_SPRITES + (model->ship.safe_to_land ? 1 : 0));
  }

#ifdef USEPSGLIB
  if (thrust_level == 0)
  {
    PSGSFXStop();
  }
  else
  {
    thrust_sfx.volume = ~(0x0f & (thrust_level - 1));
    thrust_sfx.tone = 0xc0 | (0x0f & (thrust_level - 1));
    if (PSGSFXGetStatus() != PSG_PLAYING)
      PSGSFXPlayLoop(&thrust_sfx, SFX_CHANNEL3);
  }
#endif

  static uint8_t pad_tile = 0;
  SMS_setTileatXY(model->pad_pos, 24 - model->landscape[model->pad_pos].view_y, 3 + pad_tile / 50);
  pad_tile++;
  if (pad_tile >= 250)
    pad_tile = 0;

  static int pause_count = 0;
  if (model->paused)
  {
    if (pause_count == 0)
    {
      flash_text();
      pause_count = 64;
    }
    pause_count--;
  }

  return false;
}
