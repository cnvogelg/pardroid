#include "types.h"
#include "arch.h"
#include "debug.h"
#include "system.h"

#include "proto.h"
#include "proto_low.h"
#include "proto_shared.h"
#include "func.h"

static u16 regaddr;
static u08 offslot;

// --- registers ---

void func_regaddr_set(u16 *valp)
{
  DS("ras:");
  regaddr = *valp;
  DW(regaddr); DC('.'); DNL;
}

void func_regaddr_get(u16 *valp)
{
  DS("rag:"); DW(regaddr);
  *valp = regaddr;
  DC('.'); DNL;
}

void func_reg_write(u16 *valp)
{
  // master wants to write a u16
  DS("rw:");
  u16 val = *valp;
  DW(regaddr); DC('='); DW(val);
  func_api_set_reg(regaddr & 0xff, val);
  DC('.'); DNL;
}

void func_reg_read(u16 *valp)
{
  // master wants to reead a u16
  DS("rr:"); DW(regaddr); DC('=');
  u16 val = func_api_get_reg(regaddr & 0xff);
  DW(val);
  *valp = val;
  DC('.'); DNL;
}

// --- offsets ---

void func_offslot_set(u16 *valp)
{
  offslot = (u08)(*valp);
  DS("Ow:"); DB(offslot); DNL;
}

void func_offslot_get(u16 *valp)
{
  *valp = offslot;
  DS("Or:"); DB(offslot); DNL;
}

void func_offset_set(u32 *valp)
{
  u32 offset = *valp;
  DS("os:"); DL(offset); DNL;
  func_api_set_offset(offslot, offset);
}

void func_offset_get(u32 *valp)
{
  u32 offset = func_api_get_offset(offslot);
  DS("og:"); DL(offset); DNL;
  *valp = offset;
}

static void func_handle_word(rom_pchar ptr, u08 flags)
{
  func_word_t func = (func_word_t)ptr;

  // set func
  if(flags & FUNC_FLAG_SET) {
    u16 val = proto_low_write_word();
    func(&val);
  } else {
    u16 val = 0;
    func(&val);
    proto_low_read_word(val);
  }
}

static void func_handle_long(rom_pchar ptr, u08 flags)
{
  func_long_t func = (func_long_t)ptr;

  // set func
  if(flags & FUNC_FLAG_SET) {
    u32 val = proto_low_write_long();
    func(&val);
  } else {
    u32 val = 0;
    func(&val);
    proto_low_read_long(val);
  }
}

// make long processing optional for bootloader
void func_handle_word_weak(rom_pchar ptr, u08 flags) __attribute__ ((weak, alias("func_handle_word")));
void func_handle_long_weak(rom_pchar ptr, u08 flags) __attribute__ ((weak, alias("func_handle_long")));

void func_handle(u08 num)
{
  u08 max = read_rom_char(&func_table_size);
  if(num >= max) {
    DS("f:??"); DNL;
    // wait for invalid action to time out
    proto_low_wait_cflg_hi();
    return;
  } else {
    u08 flags = read_rom_char(&func_table[num].flags);

    // get func ptr
    rom_pchar ptr = read_rom_rom_ptr(&func_table[num].func);

    // long operation
    if(flags & FUNC_FLAG_LONG) {
      func_handle_long_weak(ptr, flags);
    } else {
      func_handle_word_weak(ptr, flags);
    }

    // end function
    u08 status = proto_api_get_end_status();
    proto_low_end(status);
  }
}

void proto_api_function(u08 num) __attribute__ ((weak, alias("func_handle")));
