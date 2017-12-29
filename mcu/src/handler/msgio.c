#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_MSGIO

#include "debug.h"
#include "handler.h"
#include "buffer.h"

static u08 *data;

u08 *msgio_read_msg_prepare(u08 chn, u16 *ret_size, u16 *extra)
{
  DS("[R:");
  data = 0;
  *ret_size = 0;

  /* is valid? */
  u08 num = HANDLER_GET_TABLE_SIZE();
  if(chn >= num) {
    DC('?');
    return 0;
  }

  /* try to allocate MTU buffer */
  handler_data_t *hdata = HANDLER_GET_DATA(chn);
  u16 mtu_size = hdata->mtu;
  DW(mtu_size); DC('>');
  data = buffer_alloc(mtu_size);
  if(data == 0) {
    DC('O');
    handler_set_status(chn, HANDLER_NO_MEMORY);
    return 0;
  }

  /* let the handler fill it */
  u16 size = mtu_size;
  u08 status = handler_read(chn, &size, data);
  DW(size); DC('@'); DP(data); DC(':'); DB(status);
  handler_set_status(chn, status);
  if(status == HANDLER_OK) {
    /* shrink buffer */
    if(size < mtu_size) {
      buffer_shrink(data, size);
    }
    /* buffer ok */
    *ret_size = size;
    return data;
  }

  /* cleanup buffer on error */
  DC('!');
  buffer_free(data);
  data = 0;
  return 0;
}

void msgio_read_msg_done(u08 chn)
{
  if(data != 0) {
    /* free buffer */
    buffer_free(data);
  }
  DC(']'); DNL;
}

u08 *msgio_write_msg_prepare(u08 chn, u16 *max_size)
{
  DS("[W+:");
  data = 0;
  *max_size = 0;

  /* is valid? */
  u08 num = HANDLER_GET_TABLE_SIZE();
  if(chn >= num) {
    DC('?');
    return 0;
  }

  /* try to allocate MTU buffer */
  handler_data_t *hdata = HANDLER_GET_DATA(chn);
  u16 mtu_size = hdata->mtu;
  DW(mtu_size); DC('>');
  data = buffer_alloc(mtu_size);
  if(data == 0) {
    DC('O');
    handler_set_status(chn, HANDLER_NO_MEMORY);
    return 0;
  }

  /* return valid buffer */
  *max_size = mtu_size;
  DC(']');
  return data;
}

void msgio_write_msg_done(u08 chn, u16 size, u16 extra)
{
  DS("[W-:");
  if(data != 0) {
    DW(size);
    u08 status = handler_write(chn, size, data);
    DC('@'); DP(data); DC(':'); DB(status);
    handler_set_status(chn, status);

    /* free buffer */
    buffer_free(data);
    data = 0;
  }
  DC(']'); DNL;
}


/* aliases for proto API functions */
u08 *proto_api_read_msg_prepare(u08 chn, u16 *size, u16 *extra) __attribute__ ((weak, alias("msgio_read_msg_prepare")));
void proto_api_read_msg_done(u08 chn) __attribute__ ((weak, alias("msgio_read_msg_done")));
u08 *proto_api_write_msg_prepare(u08 chn, u16 *max_size) __attribute__ ((weak, alias("msgio_write_msg_prepare")));
void proto_api_write_msg_done(u08 chn, u16 size, u16 extra) __attribute__ ((weak, alias("msgio_write_msg_done")));
