#include "types.h"
#include "arch.h"
#include "autoconf.h"

#include "handler.h"
#include "hnd_echo.h"
#include "buffer.h"
#include "channel.h"

static u08 *data;
static u16  size;

static u08 echo_init(u08 chn)
{
  data = 0;
  size = 0;
  return HANDLER_OK;
}

static u08 *echo_read_msg_prepare(u08 chn, u16 *ret_size)
{
  /* return last written buffer */
  *ret_size = size;
  return data;
}

static void echo_read_msg_done(u08 chn)
{
  /* free buffer */
  if(data != 0) {
    buffer_free(data);
    data = 0;
    size = 0;
  }
}

static u08 *echo_write_msg_prepare(u08 chn, u16 *max_size)
{
  u16 mtu = channel_get_mtu(chn);
  u08 *buffer = buffer_alloc(mtu);
  if(buffer != 0) {
    *max_size = mtu;
    return buffer;
  } else {
    *max_size = 0;
    return 0;
  }
}

static void echo_write_msg_done(u08 chn, u16 got_size)
{
  /* adjust read size */
  size = got_size;
}

HANDLER_BEGIN(echo)
  .init_func = echo_init,
  .read_msg_prepare = echo_read_msg_prepare,
  .read_msg_done = echo_read_msg_done,
  .write_msg_prepare = echo_write_msg_prepare,
  .write_msg_done = echo_write_msg_done,
  .mtu_max = CONFIG_BUFFER_SIZE,
  .mtu_min = 2
HANDLER_END
