#include <stdbool.h>

#define LEVEL_CODE_LEN 8
#define DEFAULT_START_LEVEL "APOLIMO"

typedef struct
{
  char level_code[LEVEL_CODE_LEN];
  bool invert_controls;
  int difficulty;
} game_config_t;

void game_reset(void);
void game_run(game_config_t *config);
void game_save_config(game_config_t *config);
bool game_load_config(game_config_t *config);
