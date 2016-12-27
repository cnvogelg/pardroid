/*#include "autoconf.h"*/
#include <avr/io.h>
#include "pario_pins.S"

    .section    text

    .global  pario_init
pario_init:
    sbi _SFR_IO_ADDR(DDRD), 1
    ret
