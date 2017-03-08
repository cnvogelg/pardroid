#include "types.h"
#include "arch.h"
#include "reg_ro.h"

u16 reg_ro_get(u08 num)
{
  u08 max = reg_ro_size();
  if(num >= max) {
    return 0;
  } else {
    return read_rom_word(&reg_ro_table[num]);
  }
}

