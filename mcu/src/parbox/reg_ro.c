#include "types.h"
#include "arch.h"
#include "reg_ro.h"

#include "uartutil.h"
#include "uart.h"

u16 reg_ro_get(u08 num)
{
  u08 max = read_rom_char(&reg_ro_table_size);
  if(num >= max) {
    /* invalid reg */
    return 0;
  } else {
    u16 ptr = read_rom_word(&reg_ro_table[num].ptr);
    u08 flags = read_rom_char(&reg_ro_table[num].flags);
    if(flags & REG_RO_FLAG_FUNC) {
        reg_ro_func_t f = (reg_ro_func_t)ptr;
        return f();
    }
    else if(flags & REG_RO_FLAG_RAM) {
        if(flags & REG_RO_FLAG_BYTE) {
            u08 *vptr = (u08 *)ptr;
            return *vptr;
        } else {
            u16 *vptr = (u16 *)ptr;
            return *vptr;
        }
    }
    else {
        if(flags & REG_RO_FLAG_BYTE) {
            u08 val = read_rom_char(ptr);
            return val;
        } else {
            u16 val = read_rom_word(ptr);
            return val;
        }
    }
    return 0;
  }
}

u16 proto_api_get_ro_reg(u08 num) __attribute__ ((weak, alias("reg_ro_get")));
