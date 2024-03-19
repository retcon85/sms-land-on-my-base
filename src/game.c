#include "SMSlib.h"
#include "game_model.h"
#include "game_view.h"

static game_model_t model;

void game_loop(char *start_level)
{
  game_model_reset(&model, start_level);
  game_view_init(&model);
  while (true)
  {
    SMS_waitForVBlank();
    if (game_view_update(&model))
    {
      if (model.landed)
        game_model_reset(&model, model.level_code);
      else
        game_model_restart_level(&model);
      game_view_init(&model);
      continue;
    }

    game_model_tick(&model);
    unsigned int keys = SMS_getKeysHeld();
    if (keys & (PORT_A_KEY_1 | PORT_A_KEY_2))
      game_model_engage_thrust(&model);
    else
      game_model_disengage_thrust(&model);

    if (keys & (PORT_A_KEY_LEFT))
      game_model_turn_left(&model);
    else if (keys & (PORT_A_KEY_RIGHT))
      game_model_turn_right(&model);
  }
}
