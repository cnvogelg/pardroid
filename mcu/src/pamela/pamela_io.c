#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "debug.h"

#include "proto_io.h"
#include "proto_io_shared.h"
#include "pamela.h"
#include "pamela_int.h"

// proto_io API implementation

u16 proto_io_api_get_default_mtu(void)
{
  return PAMELA_DEFAULT_MTU;
}

u16 proto_io_api_get_max_channels(void)
{
  return PAMELA_NUM_CHANNELS;
}

u16 proto_io_api_get_channel_mtu(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  return pc->mtu;
}

void proto_io_api_set_channel_mtu(u08 chn, u16 mtu)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  // check for max
  u16 max_mtu = pc->service->handler->config.max_mtu;
  if(mtu > max_mtu) {
    mtu = max_mtu;
  }

  // set default?
  if(mtu == 0) {
    mtu = pc->service->handler->config.def_mtu;
  }

  // ask handler about new mtu
  mtu = pc->service->handler->set_mtu(chn, mtu);
  pc->mtu = mtu;
}

void proto_io_api_open(u08 chn, u16 port)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  // find service for port
  pamela_service_t *srv = pamela_find_service(port);
  if(srv == NULL) {
    // error
    pc->status = PROTO_IO_STATUS_ERROR;
  } else {
    // setup channel
    pc->status = PROTO_IO_STATUS_OPEN;
    pc->service = srv;
    pc->port = port;
    pc->mtu = srv->handler->config.def_mtu;

    // handler open
    u08 status = srv->handler->open(chn, port);
    if(status != PAMELA_OK) {
      pc->status |= PROTO_IO_STATUS_ERROR;
    } else {
      // set service active
      srv->channels |= 1<<chn;
    }
  }

  proto_io_event_mask_add_chn(chn);
}

void proto_io_api_close(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  // clear channel
  pc->service->channels &= ~(1<<chn);

  pc->status = 0;
  pc->service = 0;
  pc->port = 0;

  // call handler->close()
  pc->service->handler->close(chn);

  proto_io_event_mask_add_chn(chn);
}

void proto_io_api_reset(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  // reset status
  pc->status = 0;

  pc->service->handler->reset(chn);

  proto_io_event_mask_add_chn(chn);
}

void proto_io_api_seek(u08 chn, u32 off)
{
}

u32 proto_io_api_tell(u08 chn)
{
  return 0;
}

u16  proto_io_api_status(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  return pc->status;
}

// read

void proto_io_api_read_req(u08 chn, u16 size)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  u08 result = pc->service->handler->read_request(chn, size);
  if(result == PAMELA_OK) {
    pc->status |= PROTO_IO_STATUS_READ_REQ;
  } else {
    pc->status |= PROTO_IO_STATUS_READ_ERROR;
  }

  proto_io_event_mask_add_chn(chn);
}

u16  proto_io_api_read_res(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  return pc->rx_size;
}

void proto_io_api_read_blk(u08 chn, u16 *size, u08 **buf)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  *size = pc->rx_size;
  *buf = pc->rx_buf;

  // clear read status
  pc->status &= ~PROTO_IO_STATUS_READ_MASK;

  proto_io_event_mask_add_chn(chn);
}

void proto_io_api_read_done(u08 chn, u16 size, u08 *buf)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  pc->service->handler->read_done(chn, buf, size);
}

// write

void proto_io_api_write_req(u08 chn, u16 size)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  u08 result = pc->service->handler->write_request(chn, size);
  if(result == PAMELA_OK) {
    pc->status |= PROTO_IO_STATUS_WRITE_REQ;
  } else {
    pc->status |= PROTO_IO_STATUS_WRITE_ERROR;
  }

  proto_io_event_mask_add_chn(chn);
}

u16  proto_io_api_write_res(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  return pc->tx_size;
}

void proto_io_api_write_blk(u08 chn, u16 *size, u08 **buf)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  *size = pc->rx_size;
  *buf = pc->rx_buf;

  // clear read status
  pc->status &= ~PROTO_IO_STATUS_WRITE_MASK;

  proto_io_event_mask_add_chn(chn);
}

void proto_io_api_write_done(u08 chn, u16 size, u08 *buf)
{
  pamela_channel_t *pc = pamela_get_channel(chn);
  pc->service->handler->write_done(chn, buf, size);
}
