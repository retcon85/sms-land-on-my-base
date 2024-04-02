#include "game_model.h"
#include "random.h"

/* look up table for trig */
static const int16_t lut[] = {
    512, // cos(0) or sin(90)
    511, // cos(1) or sin(89)
    511, // cos(2) or sin(88)
    511, // cos(3) or sin(87)
    510, // cos(4) or sin(86)
    510, // cos(5) or sin(85)
    509, // cos(6) or sin(84)
    508, // cos(7) or sin(83)
    507, // cos(8) or sin(82)
    505, // cos(9) or sin(81)
    504, // cos(10) or sin(80)
    502, // cos(11) or sin(79)
    500, // cos(12) or sin(78)
    498, // cos(13) or sin(77)
    496, // cos(14) or sin(76)
    494, // cos(15) or sin(75)
    492, // cos(16) or sin(74)
    489, // cos(17) or sin(73)
    486, // cos(18) or sin(72)
    484, // cos(19) or sin(71)
    481, // cos(20) or sin(70)
    477, // cos(21) or sin(69)
    474, // cos(22) or sin(68)
    471, // cos(23) or sin(67)
    467, // cos(24) or sin(66)
    464, // cos(25) or sin(65)
    460, // cos(26) or sin(64)
    456, // cos(27) or sin(63)
    452, // cos(28) or sin(62)
    447, // cos(29) or sin(61)
    443, // cos(30) or sin(60)
    438, // cos(31) or sin(59)
    434, // cos(32) or sin(58)
    429, // cos(33) or sin(57)
    424, // cos(34) or sin(56)
    419, // cos(35) or sin(55)
    414, // cos(36) or sin(54)
    408, // cos(37) or sin(53)
    403, // cos(38) or sin(52)
    397, // cos(39) or sin(51)
    392, // cos(40) or sin(50)
    386, // cos(41) or sin(49)
    380, // cos(42) or sin(48)
    374, // cos(43) or sin(47)
    368, // cos(44) or sin(46)
    362, // cos(45) or sin(45)
    355, // cos(46) or sin(44)
    349, // cos(47) or sin(43)
    342, // cos(48) or sin(42)
    335, // cos(49) or sin(41)
    329, // cos(50) or sin(40)
    322, // cos(51) or sin(39)
    315, // cos(52) or sin(38)
    308, // cos(53) or sin(37)
    300, // cos(54) or sin(36)
    293, // cos(55) or sin(35)
    286, // cos(56) or sin(34)
    278, // cos(57) or sin(33)
    271, // cos(58) or sin(32)
    263, // cos(59) or sin(31)
    256, // cos(60) or sin(30)
    248, // cos(61) or sin(29)
    240, // cos(62) or sin(28)
    232, // cos(63) or sin(27)
    224, // cos(64) or sin(26)
    216, // cos(65) or sin(25)
    208, // cos(66) or sin(24)
    200, // cos(67) or sin(23)
    191, // cos(68) or sin(22)
    183, // cos(69) or sin(21)
    175, // cos(70) or sin(20)
    166, // cos(71) or sin(19)
    158, // cos(72) or sin(18)
    149, // cos(73) or sin(17)
    141, // cos(74) or sin(16)
    132, // cos(75) or sin(15)
    123, // cos(76) or sin(14)
    115, // cos(77) or sin(13)
    106, // cos(78) or sin(12)
    97,  // cos(79) or sin(11)
    88,  // cos(80) or sin(10)
    80,  // cos(81) or sin(9)
    71,  // cos(82) or sin(8)
    62,  // cos(83) or sin(7)
    53,  // cos(84) or sin(6)
    44,  // cos(85) or sin(5)
    35,  // cos(86) or sin(4)
    26,  // cos(87) or sin(3)
    17,  // cos(88) or sin(2)
    8,   // cos(89) or sin(1)
    0    // cos(90) or sin(0)
};

void game_model_reset(game_model_t *m, char *code, int difficulty)
{
  if (!random_deserialize_seed(code))
  {
    m->collision = true;
    return;
  }
  m->gravity = 1 + (0x07 & random_next());
  m->wind_speed = (uint8_t)random_next();
  m->safe_landing_speed = -SAFE_LANDING_SPEED * (3 - difficulty);
  m->landing_margin = LANDING_MARGIN * (3 - difficulty);
  m->fuel_consumption = difficulty + 1;

  // generate landscape
  do
  {
    m->pad_pos = ((uint8_t)random_next()) >> 3;
  } while (m->pad_pos == 31 || m->pad_pos == 0);

  uint8_t height = 5;
  uint32_t model_x = 0;
  uint16_t rnd;
  for (int i = 0; i < LANDSCAPE_WIDTH; i++)
  {
    m->landscape[i].model_x = model_x;
    model_x += (SCREEN_WIDTH / LANDSCAPE_WIDTH) * SCALE;
    m->landscape[i].view_y = height;
    m->landscape[i].model_y = height * (SCREEN_WIDTH / LANDSCAPE_WIDTH) * SCALE;
    m->landscape[i].gradient = 0;
    if (i == m->pad_pos)
      continue;
    if (height <= LANDSCAPE_MIN_HEIGHT)
      m->landscape[i].gradient = 1;
    else if (height >= LANDSCAPE_MAX_HEIGHT)
      m->landscape[i].gradient = -1;
    else
    {
      rnd = random_next();
      if (rnd > 0xaaaa)
        m->landscape[i].gradient = 1;
      else if (rnd > 0x5555)
        m->landscape[i].gradient = -1;
    }

    height += m->landscape[i].gradient;
  }

  // store level code for later
  m->level_code = code;
  random_serialize(m->level_code);

  game_model_restart_level(m);
}

void game_model_restart_level(game_model_t *m)
{
  // place ship
  m->ship.x = 16 * SCALE;
  m->ship.y = 168 * SCALE;
  m->ship.vx = 0;
  m->ship.vy = 0;
  m->ship.thrust = 0;
  m->ship.oob = false;
  m->ship.angle = 0;
  m->landed = false;
  m->collision = false;
  m->ship.fuel = UINT16_MAX;
  m->paused = false;
}

void game_model_engage_thrust(game_model_t *m)
{
  if (m->ship.thrust >= MAX_THRUST)
    return;
  m->ship.thrust++;
}

void game_model_disengage_thrust(game_model_t *m)
{
  if (m->ship.thrust == 0)
    return;
  m->ship.thrust--;
}

void game_model_decrease_angle(game_model_t *m)
{
  if (m->ship.angle == 0)
    m->ship.angle = 359;
  else
    m->ship.angle--;
}

void game_model_increase_angle(game_model_t *m)
{
  if (m->ship.angle == 359)
    m->ship.angle = 0;
  else
    m->ship.angle++;
}

static inline bool game_detect_landing(game_model_t *m)
{
  int32_t delta_x = m->pad_pos * (SCREEN_WIDTH / LANDSCAPE_WIDTH) * SCALE - m->ship.x;
  return delta_x <= m->landing_margin && delta_x >= -m->landing_margin;
}

// collision detection is moderately optimised; assumes the ship is an 8x8 sprite
static inline bool game_detect_collision(game_model_t *m)
{
  int32_t delta_y;
  int32_t delta_x;
  int section = m->ship.x / (SCALE * (SCREEN_WIDTH / LANDSCAPE_WIDTH));
  int8_t gradient = m->landscape[section].gradient;

  // if the landscape is level, the test is easy
  if (gradient == 0)
    return m->ship.y < m->landscape[section].model_y;

  delta_x = m->landscape[section].model_x - m->ship.x;

  // if the landscape goes downhill, check the left foot
  if (gradient < 0)
  {
    delta_y = m->ship.y - m->landscape[section].model_y;
    if (delta_y > 0)
      return false;
    delta_y += delta_x * gradient;
    return delta_y <= 0;
  }
  // if the landscape goes uphill, check the right foot
  section++;
  if (section >= LANDSCAPE_WIDTH)
    section = 0;
  delta_y = m->ship.y - m->landscape[section].model_y;
  if (delta_y > 0)
    return false;
  // compensate for suspected rounding errors on uphill slope
  delta_y += (delta_x - 1) * gradient;
  return delta_y <= 0;
}

void game_model_tick(game_model_t *m)
{
  if (m->collision)
    return;

  if (m->ship.fuel <= MAX_THRUST)
    m->ship.thrust = 0;

  m->ship.x += m->ship.vx + m->wind_speed / WIND_SCALE;
  m->ship.y += m->ship.vy;

  m->ship.oob = m->ship.y > LIMIT_Y;
  while (m->ship.x > LIMIT_X)
  {
    if (m->ship.x < 2 * LIMIT_X)
      m->ship.x -= LIMIT_X;
    else
      m->ship.x += LIMIT_X;
  }

  if (m->ship.angle > 270)
  {
    m->ship.angle_x = -1 * lut[m->ship.angle - 270];
    m->ship.angle_y = lut[360 - m->ship.angle];
  }
  else if (m->ship.angle > 180)
  {
    m->ship.angle_x = -1 * lut[270 - m->ship.angle];
    m->ship.angle_y = -1 * lut[m->ship.angle - 180];
  }
  else if (m->ship.angle > 90)
  {
    m->ship.angle_x = lut[m->ship.angle - 90];
    m->ship.angle_y = -1 * lut[180 - m->ship.angle];
  }
  else
  {
    m->ship.angle_x = lut[90 - m->ship.angle];
    m->ship.angle_y = lut[m->ship.angle];
  }
  m->ship.vx += (int32_t)m->ship.thrust * m->ship.angle_x / VELOCITY_SCALE;
  m->ship.vy += (int32_t)m->ship.thrust * m->ship.angle_y / VELOCITY_SCALE - m->gravity;

  m->collision = game_detect_collision(m);
  m->ship.safe_to_land = false;
  if (game_detect_landing(m))
  {
    m->ship.safe_to_land = m->ship.vy >= m->safe_landing_speed; // && m->ship.vy <= SAFE_LANDING_SPEED;
    if (m->collision)
      m->landed = m->ship.safe_to_land;
  }

  m->ship.fuel -= m->ship.thrust * m->fuel_consumption;
}
