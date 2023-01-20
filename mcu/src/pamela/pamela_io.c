#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG CONFIG_DEBUG_PAMELA

#include "debug.h"

#include "proto_io.h"
#include "proto/wire_io.h"
#include "pamela.h"
#include "pamela_int.h"

HANDLER_TABLE_DECLARE

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
  pamela_handler_ptr_t handler = HANDLER_TABLE_GET_ENTRY(pc->service->srv_id);
  DS("set_chn_mtu:"); DB(chn); DC(':'); DW(mtu);

  // check for max
  u16 max_mtu = HANDLER_GET_MAX_MTU(handler);
  if(mtu > max_mtu) {
    mtu = max_mtu;
  }

  // set default?
  if(mtu == 0) {
    mtu = HANDLER_GET_DEF_MTU(handler);
  }

  // ask handler about new mtu
  hnd_set_mtu_func_t set_mtu_func = HANDLER_FUNC_SET_MTU(handler);
  if(set_mtu_func != NULL) {
    mtu = set_mtu_func(chn, mtu);
  }

  // store mtu
  pc->mtu = mtu;
  DC('='); DW(mtu); DNL;
}

u16 proto_io_api_get_channel_error(u08 chn)
{
  DS("[error:"); DB(chn); DC(':');
  pamela_channel_t *pc = pamela_get_channel(chn);
  u16 error = pc->error;
  DW(error); DC(']'); DNL;
  return error;
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
    pamela_set_open_error(pc, PAMELA_WIRE_ERROR_NO_SERVICE);
    // notify host
    proto_io_event_mask_add_chn(pc->chan_id);
  } else {
    // try to find a slot in service for this new channel
    u08 slot_id = pamela_find_slot(srv);
    if(slot_id != PAMELA_EMPTY_SLOT) {
      // setup channel
      pc->service = srv;
      pc->port = port;
      pamela_handler_ptr_t handler = HANDLER_TABLE_GET_ENTRY(srv->srv_id);
      pc->mtu = HANDLER_GET_DEF_MTU(handler);
      pc->slot_id = slot_id;
      DC('@'); DB(slot_id); DC(',');
      DW(pc->port); DC(','); DW(pc->mtu); DC(',');

      // handler open
      pamela_open_work(pc, handler, PAMELA_CALL_FIRST);

      DNL;
    } else {
      DS("no slot!"); DNL;
      pamela_set_open_error(pc, PAMELA_WIRE_ERROR_NO_SLOT);
      // notify host
      proto_io_event_mask_add_chn(pc->chan_id);
    }
  }
}

void proto_io_api_close(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  pamela_handler_ptr_t handler = HANDLER_TABLE_GET_ENTRY(pc->service->srv_id);
  DS("close:"); DB(chn); DC(':'); DW(pc->status); DC(':');

  // if closed after an open error then do not call close func
  if((pc->status & PAMELA_STATUS_OPEN_ERROR)!=0) {
    pamela_close_work(pc, NULL, PAMELA_CALL_FIRST);
  } else {
    // call handler->close()
    pamela_close_work(pc, handler, PAMELA_CALL_FIRST);
  }

  DNL;
}

void proto_io_api_reset(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  pamela_handler_ptr_t handler = HANDLER_TABLE_GET_ENTRY(pc->service->srv_id);

  DS("reset:"); DB(chn); DC(':');

  pamela_reset_work(pc, handler, PAMELA_CALL_FIRST);

  DNL;
}

void proto_io_api_seek(u08 chn, u32 off)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  pamela_handler_ptr_t handler = HANDLER_TABLE_GET_ENTRY(pc->service->srv_id);

  DS("seek:"); DB(chn); DC(':');

  hnd_seek_func_t seek_func = HANDLER_FUNC_SEEK(handler);
  if(seek_func != NULL) {
    seek_func(chn, off);
  }

  DNL;
}

u32 proto_io_api_tell(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  pamela_handler_ptr_t handler = HANDLER_TABLE_GET_ENTRY(pc->service->srv_id);

  DS("tell:"); DB(chn); DC(':');

  hnd_tell_func_t tell_func = HANDLER_FUNC_TELL(handler);
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

// ---------- read ----------

static void setup_next_blk(pamela_buf_t *buf, u16 mtu)
{
  buf->offset += buf->blk_size;
  if(buf->offset < buf->size) {
    u16 left = buf->size - buf->offset;
    if(left <= mtu) {
      buf->blk_size = left;
    } else {
      buf->blk_size = mtu;
    }
  } else {
    buf->blk_size = 0;
  }
}

void proto_io_api_read_req(u08 chn, u16 size)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  pamela_handler_ptr_t handler = HANDLER_TABLE_GET_ENTRY(pc->service->srv_id);
  pamela_buf_t *buf = &pc->rx_buf;

  /* setup rx buf */
  buf->size = size;
  buf->offset = 0;
  buf->blk_size = 0;

  pc->rx_org_size = size;
  pc->status |= PAMELA_STATUS_READ_PRE;

  DS("[RR:"); DB(chn); DC(':'); DW(size);
  pamela_read_pre_work(pc, handler, PAMELA_CALL_FIRST);
  DC(']'); DNL;
}

u16 proto_io_api_read_res(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  u16 size = pc->rx_buf.size;
  DS("[RS:"); DB(chn); DC('='); DW(size); DC(']'); DNL;
  return size;
}

void proto_io_api_read_blk(u08 chn, u16 *size, u08 **raw_buf)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  pamela_buf_t *buf = &pc->rx_buf;

  *size = buf->blk_size;
  *raw_buf = buf->data + buf->offset;

  DS("[RB:"); DB(chn); DC('='); DW(buf->size); DC('+'); DW(buf->blk_size); DC('@'); DW(buf->offset); DC(']'); DNL;

  setup_next_blk(buf, pc->mtu);
}

void proto_io_api_read_done(u08 chn, u16 size, u08 *buf)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  pamela_handler_ptr_t handler = HANDLER_TABLE_GET_ENTRY(pc->service->srv_id);

  // was last block
  if(pc->rx_buf.blk_size == 0) {
    pc->status |= PAMELA_STATUS_READ_POST;

    DS("[RD:"); DB(chn); DC('+'); DW(pc->rx_buf.blk_size); DC('@'); DW(pc->rx_buf.offset);
    pamela_read_post_work(pc, handler, PAMELA_CALL_FIRST);
    DC(']'); DNL;
  }
}

// ---------- write ----------

void proto_io_api_write_req(u08 chn, u16 size)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  pamela_handler_ptr_t handler = HANDLER_TABLE_GET_ENTRY(pc->service->srv_id);
  pamela_buf_t *buf = &pc->tx_buf;

  /* setup tx buf */
  buf->size = size;
  buf->offset = 0;
  buf->blk_size = 0;

  pc->tx_org_size = size;
  pc->status |= PAMELA_STATUS_WRITE_PRE;

  DS("[WR:"); DB(chn); DC(':'); DW(size);
  pamela_write_pre_work(pc, handler, PAMELA_CALL_FIRST);
  DC(']'); DNL;
}

u16  proto_io_api_write_res(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  u16 size = pc->tx_buf.size;
  DS("[WS:"); DB(chn); DC('='); DW(size); DC(']'); DNL;
  return size;
}

void proto_io_api_write_blk(u08 chn, u16 *size, u08 **raw_buf)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  pamela_buf_t *buf = &pc->tx_buf;

  *size = buf->blk_size;
  *raw_buf = buf->data + buf->offset;

  DS("[WB:"); DB(chn); DC('='); DW(buf->size); DC('+'); DW(buf->blk_size); DC('@'); DW(buf->offset); DC(']'); DNL;

  setup_next_blk(buf, pc->mtu);
}

void proto_io_api_write_done(u08 chn, u16 size, u08 *buf)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  pamela_handler_ptr_t handler = HANDLER_TABLE_GET_ENTRY(pc->service->srv_id);

  // was last block
  if(pc->tx_buf.blk_size == 0) {
    pc->status |= PAMELA_STATUS_WRITE_POST;

    DS("[WD:"); DB(chn); DC('+'); DW(pc->tx_buf.blk_size); DC('@'); DW(pc->tx_buf.offset);
    pamela_write_post_work(pc, handler, PAMELA_CALL_FIRST);
    DC(']'); DNL;
  }
}
