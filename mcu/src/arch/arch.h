#ifndef ARCH_H
#define ARCH_H

#include <avr/pgmspace.h>

typedef PGM_P rom_pchar;
#define read_rom_pchar pgm_read_byte_near

#endif
