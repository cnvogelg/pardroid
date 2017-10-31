#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_HANDLER_NULL

#include "handler.h"
#include "hnd_null.h"
#include "debug.h"

static u08 null_init(u08 chn)
{
  DS("Ni"); DNL;
  return HANDLER_OK;
}

static u08 null_read(u08 chn, u16 *ret_size, u08 *buf)
{
  DS("Nr:"); DW(*ret_size); DC('@'); DP(buf); DNL;
  return HANDLER_OK;
}

static u08 null_write(u08 chn, u16 size, u08 *buf)
{
  DS("Nw:"); DW(size); DC('@'); DP(buf); DNL;
  return HANDLER_OK;
}

HANDLER_BEGIN(null)
  .init_func = null_init,
  .read_func = null_read,
  .write_func = null_write,
  .mtu_max = 0,
  .mtu_min = 0
HANDLER_END
