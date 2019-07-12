#include "types.h"
#include "arch.h"
#include "autoconf.h"

#include "proto_shared.h"
#include "func.h"
#include "status.h"

u16 func_read_word(u08 num)
{
  switch(num) {
    case PROTO_WFUNC_MAGIC:
      return PROTO_MAGIC_APPLICATION;
    default:
      return 0;
  }
}

void func_write_word(u08 num, u16 val)
{
  /* nothing to write */
}

u32 func_read_long(u08 num)
{
  switch(num) {
    case PROTO_LFUNC_STATUS:
      return status_get_mask();
    default:
      return 0;
  }
}

void func_write_long(u08 num, u32 val)
{
  /* nothing to write */
}

u16  proto_api_wfunc_read(u08 chn) __attribute__ ((weak, alias("func_read_word")));
void proto_api_wfunc_write(u08 chn, u16 val) __attribute__ ((weak, alias("func_write_word")));

u32  proto_api_lfunc_read(u08 chn) __attribute__ ((weak, alias("func_read_long")));
void proto_api_lfunc_write(u08 chn, u32 val) __attribute__ ((weak, alias("func_write_long")));
