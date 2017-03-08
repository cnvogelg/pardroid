#ifndef ARCH_H
#define ARCH_H

#include <avr/pgmspace.h>

typedef PGM_P rom_pchar;
#define read_rom_char pgm_read_byte_near
#define read_rom_word pgm_read_word_near

#define ROM_ATTR __ATTR_PROGMEM__

#endif
