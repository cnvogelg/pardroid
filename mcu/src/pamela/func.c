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

static u16 reg_addr;

// --- registers ---

u16 wfunc_read_handle(u08 num)
{
  switch(num) {
    case PROTO_WFUNC_REG_ADDR:
      return reg_addr;
    case PROTO_WFUNC_REG_VALUE:
      return proto_api_reg_read(reg_addr);
    case PROTO_WFUNC_CHAN_STATE:
      // TODO: get current channel state
      return 0;
    default:
      // unknown function
      return 0;
  }
}

void wfunc_write_handle(u08 num, u16 val)
{
  switch(num) {
    case PROTO_WFUNC_REG_ADDR:
      reg_addr = val;
      break;
    case PROTO_WFUNC_REG_VALUE:
      proto_api_reg_write(reg_addr, val);
      break;
    case PROTO_WFUNC_CHAN_STATE:
      // write ignored
      break;
    default:
      // unknown function
      break;
  }
}

u32 lfunc_read_handle(u08 num)
{
  // TODO
  return 0;
}

void lfunc_write_handle(u08 num, u32 val)
{
  // TODO
}

// bind api functions
u16  proto_api_wfunc_read(u08 num) __attribute__ ((weak, alias("wfunc_read_handle")));
void proto_api_wfunc_write(u08 num, u16 val) __attribute__ ((weak, alias("wfunc_write_handle")));
u32  proto_api_lfunc_read(u08 num) __attribute__ ((weak, alias("lfunc_read_handle")));
void proto_api_lfunc_write(u08 num, u32 val) __attribute__ ((weak, alias("lfunc_write_handle")));
