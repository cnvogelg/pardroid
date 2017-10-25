#include "types.h"
#include "arch.h"
#include "debug.h"
#include "system.h"

#include "proto.h"
#include "proto_low.h"
#include "proto_shared.h"
#include "func.h"

static u16 regaddr;

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
    func_func_t func = (func_func_t)ptr;

    // set func
    if(flags & FUNC_FLAG_SET) {
      u16 val = proto_low_write_word();
      func(&val);
    } else {
      u16 val = 0;
      func(&val);
      proto_low_read_word(val);
    }

    // end function
    u08 status = proto_api_get_end_status();
    proto_low_end(status);
  }
}

void proto_api_function(u08 num) __attribute__ ((weak, alias("func_handle")));
