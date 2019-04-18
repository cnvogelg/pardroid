#include "types.h"
#include "arch.h"
#include "autoconf.h"

#include "proto_shared.h"
#include "reg.h"
#include "func.h"
#include "chan.h"

u16 func_read_word(u08 num)
{
  switch(num) {
    case PROTO_WFUNC_MAGIC:
      return PROTO_MAGIC_APPLICATION;
    case PROTO_WFUNC_CHAN_RX_PEND:
      return chan_getclr_rx_pending();
    case PROTO_WFUNC_CHAN_ERROR:
      return chan_getclr_error();
    case PROTO_WFUNC_REG_ADDR:
      return reg_get_addr();
    case PROTO_WFUNC_REG_VALUE:
      return reg_get_value();
    default:
      return 0;
  }
}

void func_write_word(u08 num, u16 val)
{
  switch(num) {
    case PROTO_WFUNC_REG_ADDR:
      reg_set_addr(val);
      break;
    case PROTO_WFUNC_REG_VALUE:
      reg_set_value(val);
      break;
  }
}

u16  proto_api_wfunc_read(u08 chn) __attribute__ ((weak, alias("func_read_word")));
void proto_api_wfunc_write(u08 chn, u16 val) __attribute__ ((weak, alias("func_write_word")));
