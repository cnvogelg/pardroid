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

static u08 *null_read_msg_prepare(u08 chn, u16 *ret_size)
{
  DS("Nr+:"); DNL;
  *ret_size = 0x2000;
  return 0;
}

static void null_read_msg_done(u08 chn, u08 status)
{
  DS("Nr-:"); DNL;
}

static u08 *null_write_msg_prepare(u08 chn, u16 *max_size)
{
  DS("Nw+:"); DNL;
  *max_size = 0;
  return 0;
}

static void null_write_msg_done(u08 chn, u16 got_size)
{
  DS("Nw-:"); DNL;
}

HANDLER_BEGIN(null)
  .init_func = null_init,
  .read_msg_prepare = null_read_msg_prepare,
  .read_msg_done = null_read_msg_done,
  .write_msg_prepare = null_write_msg_prepare,
  .write_msg_done = null_write_msg_done,
  .mtu_max = 0,
  .mtu_min = 0
HANDLER_END
