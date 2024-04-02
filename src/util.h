#include "SMSlib.h"

void clear_screen(const char *with);
void fade_in(const void *background_palette, const void *sprite_palette, unsigned int sprite_palette_bank);
void fade_out(const void *background_palette, const void *sprite_palette, unsigned int sprite_palette_bank);
void cancelable_wait(int seconds);
void wait(int seconds);
void flash_text(void);
void flash_text_hide(void);
void print_dec1(unsigned int x, unsigned int y, int value, unsigned int pad);
void print_dec2(unsigned int x, unsigned int y, int value, unsigned int pad);
void print_dec3(unsigned int x, unsigned int y, int value, unsigned int pad);
