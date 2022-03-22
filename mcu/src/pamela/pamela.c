#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_PAMELA

#include "debug.h"
#include "proto_io.h"
#include "pamela.h"
#include "pamela_int.h"

// the channel array
static pamela_channel_t pamela_channels[PAMELA_NUM_CHANNELS];
// the mask of active channels
static u16 active_channels;

// service list
static pamela_service_t pamela_services[PAMELA_NUM_HANDLERS];
// number of added services
static u08 num_services;

void pamela_init(void)
{
  DS("pam_init"); DNL;
  proto_io_init();

  active_channels = 0;
  num_services = 0;

  // setup channels
  for(u08 i=0;i<PAMELA_NUM_CHANNELS;i++) {
    pamela_channel_t *chn = &pamela_channels[i];
    chn->status = 0;
    chn->rx_size = 0;
    chn->rx_buf = 0;
    chn->tx_size = 0;
    chn->tx_buf = 0;
    chn->service = NULL;
    chn->port = 0;
  }

  // setup services
  for(u08 i=0;i<PAMELA_NUM_HANDLERS;i++) {
    pamela_service_t *srv = &pamela_services[i];
    srv->handler = NULL;
    srv->channels = 0;
  }

  DS("pam_init done"); DNL;
}

u08 pamela_add_handler(pamela_handler_ptr_t handler)
{
  if(num_services == PAMELA_NUM_HANDLERS) {
    return PAMELA_ERROR;
  }

  pamela_service_t *srv = &pamela_services[num_services];
  srv->handler = handler;
  srv->channels = 0;

  num_services++;

  return PAMELA_OK;
}

void pamela_work(void)
{
  proto_io_handle_cmd();

  // call worker of active services
  for(u08 i=0;i<num_services;i++) {
    pamela_service_t *srv = &pamela_services[i];
    if(srv->channels != 0) {
      hnd_work_func_t work_func = HANDLER_FUNC_WORK(srv->handler);
      if(work_func != NULL) {
        work_func(srv->channels);
      }
    }
  }
}

pamela_channel_t *pamela_get_channel(u08 chn)
{
  return &pamela_channels[chn];
}

pamela_service_t *pamela_find_service(u16 port)
{
  for(u08 i=0;i<num_services;i++) {
    pamela_service_t *srv = &pamela_services[i];
    u16 port_begin = HANDLER_GET_PORT_BEGIN(srv->handler);
    u16 port_end = HANDLER_GET_PORT_END(srv->handler);
    if((port >= port_begin) && (port <= port_end)) {
      return srv;
    }
  }
  return NULL;
}

// ----- API functions for handlers to use -----

void pamela_read_reply(u08 chn, u08 *buf, u16 size)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  // size is not the requested size
  if(size != pc->rx_size) {
    pc->rx_size = size;
    pc->status |= PAMELA_STATUS_READ_SIZE;
  }

  // read is now pending
  pc->status |= PAMELA_STATUS_READ_READY;
  pc->rx_buf = buf;

  DS("[rp:"); DB(chn); DC('='); DW(size); DC(']'); DNL;

  // notify host
  proto_io_event_mask_add_chn(chn);
}

void pamela_read_error(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  pc->status |= PAMELA_STATUS_READ_ERROR;
  pc->rx_buf = NULL;
  pc->rx_size = 0;

  DS("[re:"); DB(chn); DC(']'); DNL;

  // notify host
  proto_io_event_mask_add_chn(chn);
}

void pamela_write_reply(u08 chn, u08 *buf, u16 size)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  // size is not the requested size
  if(size != pc->tx_size) {
    pc->tx_size = size;
    pc->status |= PAMELA_STATUS_WRITE_SIZE;
  }

  // read is now pending
  pc->status |= PAMELA_STATUS_WRITE_READY;
  pc->tx_buf = buf;

  DS("[wp:"); DB(chn); DC('='); DW(size); DC(']'); DNL;

  // notify host
  proto_io_event_mask_add_chn(chn);
}

void pamela_write_error(u08 chn)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  pc->status |= PAMELA_STATUS_READ_ERROR;
  pc->rx_buf = NULL;
  pc->rx_size = 0;

  DS("[we:"); DB(chn); DC(']'); DNL;

  // notify host
  proto_io_event_mask_add_chn(chn);
}

void pamela_end_stream(u08 chn, u08 error)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  if(error)
    pc->status |= PAMELA_STATUS_ERROR;
  else
    pc->status |= PAMELA_STATUS_EOS;

  DS("[eos:"); DB(chn); DC('='); DB(error); DC(']'); DNL;

  // notify host
  proto_io_event_mask_add_chn(chn);
}
