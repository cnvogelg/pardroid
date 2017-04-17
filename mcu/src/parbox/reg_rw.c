#include "types.h"
#include "arch.h"
#include "reg_rw.h"

u16 reg_rw_get(u08 num)
{
  u08 max = read_rom_char(&reg_rw_table_size);
  if(num >= max) {
    /* invalid reg */
    return 0;
  } else {
    u08 flags = read_rom_char(&reg_rw_table[num].flags);
    if(flags & REG_RW_FLAG_FUNC) {
      /* getter function */
      rom_pchar ptr = read_rom_rom_ptr(&reg_rw_table[num].get_func);
      reg_rw_get_func_t get_func = (reg_rw_get_func_t)ptr;
      return get_func();
    } else {
      rom_pchar ptr = read_rom_rom_ptr(&reg_rw_table[num].ptr);
      if(flags & REG_RW_FLAG_BYTE) {
        /* byte value */
        u08 *cptr = (u08 *)ptr;
        return *cptr;
      } else {
        /* word value */
        u16 *wptr = (u16 *)ptr;
        return *wptr;
      }
    }
    return 0;
  }
}

u16 proto_api_get_rw_reg(u08 num) __attribute__ ((weak, alias("reg_rw_get")));

void reg_rw_set(u08 num, u16 val)
{
  u08 max = read_rom_char(&reg_rw_table_size);
  if(num >= max) {
    /* invalid reg */
    return;
  } else {
    u08 flags = read_rom_char(&reg_rw_table[num].flags);
    if(flags & REG_RW_FLAG_FUNC) {
      /* setter function */
      rom_pchar ptr = read_rom_rom_ptr(&reg_rw_table[num].set_func);
      reg_rw_set_func_t set_func = (reg_rw_set_func_t)ptr;
      set_func(val);
    }
    else {
      /* set value */
      rom_pchar ptr = read_rom_rom_ptr(&reg_rw_table[num].ptr);
      if(flags & REG_RW_FLAG_BYTE) {
          u08 *cptr = (u08 *)ptr;
          *cptr = (u08)val;
      } else {
          u16 *wptr = (u16 *)ptr;
          *wptr = val;
      }
    }
  }
}

void proto_api_set_rw_reg(u08 num, u16 val) __attribute__ ((weak, alias("reg_rw_set")));
