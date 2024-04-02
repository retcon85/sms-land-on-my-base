#include "game.h"
#include "SMSlib.h"
#include "PSGlib.h"
#include "game_model.h"
#include "game_view.h"
#include "random.h"

static game_model_t model;

void game_run(game_config_t *config)
{
  game_save_config(config);
  game_model_reset(&model, config->level_code, config->difficulty);
  game_view_init(&model);
  while (true)
  {
    SMS_waitForVBlank();
    if (game_view_update(&model))
    {
      if (model.landed)
      {
        game_save_config(config);
        game_model_reset(&model, config->level_code, config->difficulty);
      }
      else
      {
        game_model_restart_level(&model);
      }
      game_view_init(&model);
      continue;
    }

    if (SMS_queryPauseRequested())
    {
      SMS_resetPauseRequest();
      model.paused = !model.paused;
    }

    if (model.paused)
    {
      PSGSFXStop();
      continue;
    }

    game_model_tick(&model);
    unsigned int keys = SMS_getKeysHeld();
    if (keys & (PORT_A_KEY_1 | PORT_A_KEY_2))
      game_model_engage_thrust(&model);
    else
      game_model_disengage_thrust(&model);

    if (keys & (config->invert_controls ? PORT_A_KEY_LEFT : PORT_A_KEY_RIGHT))
      game_model_decrease_angle(&model);
    else if (keys & (config->invert_controls ? PORT_A_KEY_RIGHT : PORT_A_KEY_LEFT))
      game_model_increase_angle(&model);
  }
}

void game_save_config(game_config_t *config)
{
  SMS_enableSRAM();
  game_config_t *save_config = (game_config_t *)SMS_SRAM;
  for (int i = 0; i < sizeof(game_config_t); i++)
    save_config[i] = config[i];
  SMS_disableSRAM();
}

bool game_load_config(game_config_t *config)
{
  for (int i = 0; i < sizeof(config->level_code); i++)
    config->level_code[i] = DEFAULT_START_LEVEL[i];
  config->difficulty = 1;
  config->invert_controls = false;

  SMS_enableSRAM();
  game_config_t *save_config = (game_config_t *)SMS_SRAM;
  bool loaded = random_deserialize_seed(save_config->level_code);
  if (loaded)
  {
    for (int i = 0; i < sizeof(game_config_t); i++)
      config[i] = save_config[i];
  }
  SMS_disableSRAM();
  return loaded;
}
