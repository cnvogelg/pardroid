#include "types.h"
#include "arch.h"
#include "reg.h"

u16 reg_get(u08 num)
{
  u08 max = read_rom_char(&reg_table_size);
  if(num >= max) {
    /* invalid reg */
    return 0;
  } else {
    u08 flags = read_rom_char(&reg_table[num].flags);
    if(flags & REG_FLAG_FUNC) {
      /* getter function */
      rom_pchar ptr = read_rom_rom_ptr(&reg_table[num].ptr.func);
      reg_func_t func = (reg_func_t)ptr;
      u16 val;
      func(&val, REG_MODE_READ);
      return val;
    } else {
      rom_pchar ptr = read_rom_rom_ptr(&reg_table[num].ptr.var);
      /* word value */
      if(flags & REG_FLAG_ROM) {
        return read_rom_word(ptr);
      } else {
        u16 *wptr = (u16 *)ptr;
        return *wptr;
      }
    }
    return 0;
  }
}

u16 func_api_get_reg(u08 num) __attribute__ ((weak, alias("reg_get")));

void reg_set(u08 num, u16 val)
{
  u08 max = read_rom_char(&reg_table_size);
  if(num >= max) {
    /* invalid reg */
    return;
  } else {
    u08 flags = read_rom_char(&reg_table[num].flags);
    if(!(flags & REG_FLAG_WRITE)) {
      /* not writeable */
      return;
    }
    if(flags & REG_FLAG_FUNC) {
      /* setter function */
      rom_pchar ptr = read_rom_rom_ptr(&reg_table[num].ptr.func);
      reg_func_t func = (reg_func_t)ptr;
      func(&val, REG_MODE_WRITE);
    }
    else {
      /* set value */
      rom_pchar ptr = read_rom_rom_ptr(&reg_table[num].ptr.var);
      u16 *wptr = (u16 *)ptr;
      *wptr = val;
    }
  }
}

void func_api_set_reg(u08 num, u16 val) __attribute__ ((weak, alias("reg_set")));
