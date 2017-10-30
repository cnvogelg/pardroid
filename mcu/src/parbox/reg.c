#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_REG

#include "debug.h"
#include "reg.h"

extern const reg_table_t * const reg_table ROM_ATTR;

static const reg_def_t *get_reg_def(u08 num)
{
  const reg_table_t *tab = reg_table;
  while(tab != 0) {
    /* scan entries */
    u08 size = read_rom_char(&tab->size);
    u08 offset = read_rom_char(&tab->offset);
    u08 last = size + offset;
    if((num >= offset) && (num < last)) {
      u08 idx = num - offset;
      return &tab->entries[idx];
    }
    tab = (const reg_table_t *)read_rom_rom_ptr(&tab->next);
  }
  return 0;
}

u16 reg_get(u08 num)
{
  const reg_def_t *def = get_reg_def(num);
  if(def == 0) {
    DS('Rr?'); DB(num); DNL;
    return 0;
  }

  u08 flags = read_rom_char(&def->flags);
  u16 val = 0;
  if(flags & REG_FLAG_FUNC) {
    /* getter function */
    rom_pchar ptr = read_rom_rom_ptr(&def->ptr.func);
    reg_func_t func = (reg_func_t)ptr;
    func(&val, REG_MODE_READ);
  } else {
    rom_pchar ptr = read_rom_rom_ptr(&def->ptr.var);
    /* word value */
    if(flags & REG_FLAG_ROM) {
      val = read_rom_word(ptr);
    } else {
      u16 *wptr = (u16 *)ptr;
      val = *wptr;
    }
  }
  DS('Rr@'); DB(num); DC('='); DW(val); DNL;
  return val;
}

void reg_set(u08 num, u16 val)
{
  const reg_def_t *def = get_reg_def(num);
  if(def == 0) {
    DS('Rw?'); DB(num); DNL;
    return;
  }

  u08 flags = read_rom_char(&def->flags);
  if(!(flags & REG_FLAG_WRITE)) {
    /* not writeable */
    DS('Rw!'); DB(num); DNL;
    return;
  }
  DS('Rw@'); DB(num); DC('='); DW(val); DNL;
  if(flags & REG_FLAG_FUNC) {
    /* setter function */
    rom_pchar ptr = read_rom_rom_ptr(&def->ptr.func);
    reg_func_t func = (reg_func_t)ptr;
    func(&val, REG_MODE_WRITE);
  }
  else {
    /* set value */
    rom_pchar ptr = read_rom_rom_ptr(&def->ptr.var);
    u16 *wptr = (u16 *)ptr;
    *wptr = val;
  }
}

/* link to func API */
u16 func_api_get_reg(u16 num) __attribute__ ((weak, alias("reg_get")));
void func_api_set_reg(u16 num, u16 val) __attribute__ ((weak, alias("reg_set")));
