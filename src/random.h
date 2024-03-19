#include <stdint.h>
#include <stdbool.h>

uint16_t random_next(void);
void random_serialize(char *str);
bool random_deserialize_seed(const char *str);
