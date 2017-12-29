#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_FUNC

#include "debug.h"
#include "system.h"

#include "proto.h"
#include "proto_low.h"
#include "proto_shared.h"
#include "func.h"

static u08 regaddr;
static u08 regladdr;

// --- registers ---

void func_regaddr_set(u16 *valp)
{
  DS("ras:");
  regaddr = (u08)(*valp & 0xff);
  DB(regaddr); DC('.'); DNL;
}

void func_regaddr_get(u16 *valp)
{
  DS("rag:"); DB(regaddr);
  *valp = regaddr;
  DC('.'); DNL;
}

void func_reg_write(u16 *valp)
{
  // master wants to write a u16
  DS("rw:");
  u16 val = *valp;
  DB(regaddr); DC('='); DW(val);
  func_api_set_reg(regaddr, val);
  DC('.'); DNL;
}

void func_reg_read(u16 *valp)
{
  // master wants to reead a u16
  DS("rr:"); DB(regaddr); DC('=');
  u16 val = func_api_get_reg(regaddr);
  DW(val);
  *valp = val;
  DC('.'); DNL;
}

// --- long registers ---

void func_regladdr_set(u16 *valp)
{
  regladdr = (u08)(*valp & 0xff);
  DS("RAS:"); DB(regladdr); DNL;
}

void func_regladdr_get(u16 *valp)
{
  *valp = regladdr;
  DS("RAG:"); DB(regladdr); DNL;
}

void func_regl_write(u32 *valp)
{
  DS("RW:");
  u32 val = *valp;
  DB(regladdr); DC('='); DL(val); DNL;
  func_api_set_regl(regladdr, val);
  DC('.'); DNL;
}

void func_regl_read(u32 *valp)
{
  DS("RR:"); DB(regladdr); DC('=');
  u32 val = func_api_get_regl(regladdr);
  DL(val);
  *valp = val;
  DC('.'); DNL;
}

static void func_handle_word(func_word_t func, u08 flags)
{
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

static void func_handle_long(func_long_t func, u08 flags)
{
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
void func_handle_word_weak(func_word_t func, u08 flags) __attribute__ ((weak, alias("func_handle_word")));
void func_handle_long_weak(func_long_t func, u08 flags) __attribute__ ((weak, alias("func_handle_long")));

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

    // long operation
    if(flags & FUNC_FLAG_LONG) {
      func_long_t func = (func_long_t)read_rom_rom_ptr(&func_table[num].func.flong);
      func_handle_long_weak(func, flags);
    } else {
      func_word_t func = (func_word_t)read_rom_rom_ptr(&func_table[num].func.fword);
      func_handle_word_weak(func, flags);
    }

    // end function
    u08 status = proto_api_get_end_status();
    proto_low_end(status);
    DS("fs:"); DB(status); DNL;
  }
}

void proto_api_function(u08 num) __attribute__ ((weak, alias("func_handle")));
