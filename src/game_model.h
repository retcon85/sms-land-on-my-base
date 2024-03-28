#ifndef __GAME_MODEL_H__
#define __GAME_MODEL_H__

#include <stdint.h>
#include <stdbool.h>

#define SCREEN_WIDTH 256
#define LIMIT_Y (200 * SCALE)
#define LIMIT_X (SCREEN_WIDTH * SCALE)
#define VELOCITY_SCALE 1024
#define WIND_SCALE 1
#define LANDSCAPE_WIDTH 32
#define SCALE 4096L
#define LANDING_MARGIN (1.5 * SCALE)
#define LANDSCAPE_MIN_HEIGHT 2
#define LANDSCAPE_MAX_HEIGHT 20
#define MAX_THRUST 32
#define SAFE_LANDING_SPEED (SCALE / 16)

typedef struct
{
  uint32_t x;
  uint32_t y;
  int32_t vx;
  int32_t vy;
  uint16_t angle;
  bool oob;
  uint8_t thrust;
  int16_t angle_x;
  int16_t angle_y;
} ship_t;

typedef struct
{
  uint8_t x;
  uint8_t y;
} pos_t;

typedef struct
{
  uint32_t model_x;
  uint32_t model_y;
  uint8_t view_y;
  int8_t gradient;
} landscape_section_t;

typedef struct
{
  landscape_section_t landscape[LANDSCAPE_WIDTH];
  uint8_t pad_pos;
  ship_t ship;
  uint8_t gravity;
  int8_t wind_speed;
  bool collision;
  bool landed;
  char *level_code;
  bool safe_to_land;
  int32_t landing_margin;
  int16_t safe_landing_speed;
} game_model_t;

void game_model_reset(game_model_t *m, char *code, int difficulty);
void game_model_restart_level(game_model_t *m);
void game_model_engage_thrust(game_model_t *m);
void game_model_disengage_thrust(game_model_t *m);
void game_model_decrease_angle(game_model_t *m);
void game_model_increase_angle(game_model_t *m);
void game_model_tick(game_model_t *m);

#endif
