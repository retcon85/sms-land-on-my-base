#include "random.h"
#include <stdint.h>
#include <stdbool.h>

static uint16_t seed = 0x49E5;

uint16_t random_next(void)
{
  uint8_t lsb = seed & 1;
  seed >>= 1;
  if (lsb == 1)
    seed ^= 0xb400u;
  return seed;
}

// str must be able to hold at least 8 characters (7 + null)
void random_serialize(char *str)
{
  // backup previous seed
  uint16_t value = seed;
  int i;
  // first four characters are just seed shifted 4 bits at a time
  for (i = 0; i < 4; i++)
  {
    str[i] = 'A' + (seed & 0x000f);
    seed >>= 4;
  }
  seed = value;
  // last three characters act as checksum and come from sequence
  for (; i < 7; i++)
  {
    random_next();
    str[i] = 'A' + (seed & 0x000f);
  }
  str[7] = '\0';

  // restore previous seed
  seed = value;
}

// 7 characters will be read from str
bool random_deserialize_seed(const char *str)
{
  // backup previous seed in case we fail checksum
  uint16_t value = seed;

  // first four characters shift the seed
  seed = 0;
  int i;
  for (i = 0; i < 4; i++)
  {
    seed <<= 4;
    seed |= 0x0f & (str[3 - i] - 'A');
  }

  // now generate three more characters and check that they match
  uint16_t maybe_seed = seed;
  for (; i < 7; i++)
  {
    random_next();
    if (str[i] != 'A' + (seed & 0x000f))
    {
      // checksum failed - restore previous seed and signal failure
      seed = value;
      return false;
    }
  }

  seed = maybe_seed;
  return true;
}
