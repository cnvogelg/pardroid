#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_HANDLER_ECHO

#include "handler.h"
#include "hnd_echo.h"
#include "buffer.h"
#include "debug.h"

static u08 *data;
static u16  data_size;

static u08 echo_init(u08 chn)
{
  DS("Ei"); DNL;
  data = 0;
  data_size = 0;
  return HANDLER_OK;
}

static u08 echo_read(u08 chn, u16 *size, u08 *buf)
{
  DS("Er:"); DW(size); DC('@'); DP(buf); DNL;
  if(data != 0) {
    if(*size == data_size) {
      for(u16 i=0;i<*size;i++) {
        buf[i] = data[i];
      }
    } else {
      *size = 0;
    }
    buffer_free(data);
    data = 0;
  } else {
    *size = 0;
  }
  return HANDLER_OK;
}

static u08 echo_write(u08 chn, u16 size, u08 *buf)
{
  DS("Ew:"); DW(size); DC('@'); DP(buf);
  data = buffer_alloc(size);
  data_size = size;
  DC('#'); DP(data);
  if(data != 0) {
    /* copy buf */
    for(u16 i=0;i<size;i++) {
      data[i] = buf[i];
    }
    return HANDLER_OK;
  } else {
    return HANDLER_NO_MEMORY;
  }
}

HANDLER_BEGIN(echo)
  .init_func = echo_init,
  .read_func = echo_read,
  .write_func = echo_write,
  .mtu_max = 512,
  .mtu_min = 2
HANDLER_END
