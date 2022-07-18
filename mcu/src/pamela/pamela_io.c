#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG CONFIG_DEBUG_PAMELA

#include "debug.h"

#include "proto_io.h"
#include "proto/wire_io.h"
#include "pamela.h"
#include "pamela_int.h"

// proto_io API implementation

u16 proto_io_api_get_default_mtu(void)
{
  DS("get_def_mtu:"); DW(PAMELA_DEFAULT_MTU); DNL;
  return PAMELA_DEFAULT_MTU;
}

u16 proto_io_api_get_max_channels(void)
{
  DS("get_max_chn:"); DW(PAMELA_NUM_CHANNELS); DC('!'); DNL;
  return PAMELA_NUM_CHANNELS;
}

u16 proto_io_api_get_channel_mtu(u08 chn)
{
  DS("get_chn_mtu:"); DB(chn); DC(':');
  pamela_channel_t *pc = pamela_get_channel(chn);
  u16 mtu = pc->mtu;
  DW(mtu); DNL;
  return mtu;
}

void proto_io_api_set_channel_mtu(u08 chn, u16 mtu)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  DS("set_chn_mtu:"); DB(chn); DC(':'); DW(mtu);

  // check for max
  u16 max_mtu = HANDLER_GET_MAX_MTU(pc->service->handler);
  if(mtu > max_mtu) {
    mtu = max_mtu;
  }

  // set default?
  if(mtu == 0) {
    mtu = HANDLER_GET_DEF_MTU(pc->service->handler);
  }

  // ask handler about new mtu
  hnd_set_mtu_func_t set_mtu_func = HANDLER_FUNC_SET_MTU(pc->service->handler);
  if(set_mtu_func != NULL) {
    mtu = set_mtu_func(chn, mtu);
  }

  // store mtu
  pc->mtu = mtu;
  DC('='); DW(mtu); DNL;
}

void proto_io_api_open(u08 chn, u16 port)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  DS("open:"); DB(chn); DC(':');

  // find service for port
  pamela_service_t *srv = pamela_find_service(port);
  if(srv == NULL) {
    // error
    DS("no srv!"); DNL;
    pamela_set_open_error(pc, PAMELA_DEV_ERROR_NO_SERVICE);
  } else {
    // try to find a slot in service for this new channel
    u08 slot_id = pamela_find_slot(srv);
    if(slot_id != PAMELA_EMPTY_SLOT) {
      // setup channel
      pc->service = srv;
      pc->port = port;
      pc->mtu = HANDLER_GET_DEF_MTU(srv->handler);
      pc->slot_id = slot_id;
      DC('@'); DB(slot_id); DC(','); DW(pc->port); DC(','); DW(pc->mtu); DC(',');

      // handler open
      hnd_open_func_t open_func = HANDLER_FUNC_OPEN(srv->handler);
      pamela_open_work(pc, open_func);

      DNL;
    } else {
      DS("no slot!"); DNL;
      pamela_set_open_error(pc, PAMELA_DEV_ERROR_NO_SLOT);
    }
  }
}

void proto_io_api_close(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  DS("close:"); DB(chn); DC(':');

  // call handler->close()
  hnd_close_func_t close_func = HANDLER_FUNC_CLOSE(pc->service->handler);
  pamela_close_work(pc, close_func);

  DNL;
}

void proto_io_api_reset(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  DS("reset:"); DB(chn); DC(':');

  hnd_reset_func_t reset_func = HANDLER_FUNC_RESET(pc->service->handler);
  pamela_reset_work(pc, reset_func);

  DNL;
}

void proto_io_api_seek(u08 chn, u32 off)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  DS("seek:"); DB(chn); DC(':');

  hnd_seek_func_t seek_func = HANDLER_FUNC_SEEK(pc->service->handler);
  if(seek_func != NULL) {
    seek_func(chn, off);
  }

  DNL;
}

u32 proto_io_api_tell(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  DS("tell:"); DB(chn); DC(':');

  hnd_tell_func_t tell_func = HANDLER_FUNC_TELL(pc->service->handler);
  u32 pos = 0;
  if(tell_func) {
    pos = tell_func(chn);
  }

  DL(pos); DNL;
  return pos;
}

u16  proto_io_api_status(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  u16 status = pc->status;
  DS("[status:"); DB(chn); DC(':'); DW(status); DC(']'); DNL;
  return status;
}

// read

void proto_io_api_read_req(u08 chn, u16 size)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  pc->rx_size = size;
  pc->rx_org_size = size;
  pc->status |= PAMELA_STATUS_READ_REQ;

  DS("[RR:"); DB(chn); DC(':'); DW(size);

  hnd_read_request_func_t read_req_func = HANDLER_FUNC_READ_REQUEST(pc->service->handler);
  pamela_read_work(pc, read_req_func);

  DC(']'); DNL;
}

u16  proto_io_api_read_res(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  u16 size = pc->rx_size;
  DS("[RS:"); DB(chn); DC('='); DW(size); DC(']'); DNL;
  return size;
}

void proto_io_api_read_blk(u08 chn, u16 *size, u08 **buf)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  *size = pc->rx_size;
  *buf = pc->rx_buf;

  DS("[RB:"); DB(chn); DC('='); DW(*size); DC(']'); DNL;

  // clear read status
  pc->status &= ~PAMELA_STATUS_READ_MASK;
  pc->rx_buf = NULL;
  pc->rx_size = 0;
}

void proto_io_api_read_done(u08 chn, u16 size, u08 *buf)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  DS("[RD:"); DB(chn); DC('='); DW(size);
  hnd_read_done_func_t read_done = HANDLER_FUNC_READ_DONE(pc->service->handler);
  if(read_done != NULL) {
    read_done(chn, buf, size);
  }
  DC(']'); DNL;
}

// write

void proto_io_api_write_req(u08 chn, u16 size)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  pc->tx_size = size;
  pc->tx_org_size = size;
  pc->status |= PAMELA_STATUS_WRITE_REQ;

  DS("[WR:"); DB(chn); DC(':'); DW(size);

  hnd_write_request_func_t write_req_func = HANDLER_FUNC_WRITE_REQUEST(pc->service->handler);
  pamela_write_work(pc, write_req_func);

  DC(']'); DNL;
}

u16  proto_io_api_write_res(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  u16 size = pc->tx_size;
  DS("[WS:"); DB(chn); DC('='); DW(size); DC(']'); DNL;
  return size;
}

void proto_io_api_write_blk(u08 chn, u16 *size, u08 **buf)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  *size = pc->tx_size;
  *buf = pc->tx_buf;

  DS("[WB:"); DB(chn); DC('='); DW(*size); DC(']'); DNL;

  // clear read status
  pc->status &= ~PAMELA_STATUS_WRITE_MASK;
  pc->tx_buf = NULL;
  pc->tx_size = 0;
}

void proto_io_api_write_done(u08 chn, u16 size, u08 *buf)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  DS("[WD:"); DB(chn); DC('='); DW(size);
  hnd_write_done_func_t write_done = HANDLER_FUNC_WRITE_DONE(pc->service->handler);
  if(write_done != NULL) {
    write_done(chn, buf, size);
  }
  DC(']'); DNL;
}
