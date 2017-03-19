#include "types.h"
#include "arch.h"
#include "reg_ro.h"

#include "uartutil.h"
#include "uart.h"

u16 reg_ro_get(u08 num)
{
  u08 max = read_rom_char(&reg_ro_ptr_table_size);
  if(num >= max) {
    return 0;
  } else {
    u16 addr = read_rom_word(&reg_ro_ptr_table[num]);
    u16 val = read_rom_word(addr);
    return val;
  }
}

