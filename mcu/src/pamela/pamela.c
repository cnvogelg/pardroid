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
    chn->chan_id = i;
    chn->slot_id = PAMELA_EMPTY_SLOT;
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

static void pamela_channel_work(pamela_channel_t *chn);

void pamela_work(void)
{
  proto_io_handle_cmd();

  // process active channels
  for(u08 i=0;i<PAMELA_NUM_CHANNELS;i++) {
    pamela_channel_t *chn = &pamela_channels[i];
    // only work in active channels
    u08 active = chn->status & PAMELA_STATUS_ACTIVE_MASK;
    if(active != 0) {
      pamela_channel_work(chn);
    }
  }
}

// ----- internal helpers -----

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

u08 pamela_find_slot(pamela_service_t *srv)
{
  u08 max_slots = HANDLER_GET_MAX_SLOTS(srv->handler);

  // try all slots
  for(u08 slot=0;slot<max_slots;slot++) {
    // check if any channel uses this slot already
    u08 found = 0;
    for(u08 i=0;i<PAMELA_NUM_CHANNELS;i++) {
      pamela_channel_t *chn = &pamela_channels[i];
      if(chn->service == srv) {
        if(chn->slot_id == slot) {
          found = 1;
          break;
        }
      }
    }

    // slot unused?
    if(!found) {
      return slot;
    }
  }
  // no slot found
  return PAMELA_EMPTY_SLOT;
}

void pamela_set_status(pamela_channel_t *pc, u08 status)
{
  pc->status &= ~PAMELA_STATUS_STATE_MASK;
  pc->status |= status;
}

void pamela_set_error(pamela_channel_t *pc, u08 error)
{
  pc->status &= ~(PAMELA_STATUS_STATE_MASK | PAMELA_STATUS_ERROR_MASK);
  pc->status |= PAMELA_STATUS_ERROR;
  pc->status |= error << PAMELA_STATUS_ERROR_SHIFT;
}

void pamela_set_open_error(pamela_channel_t *pc, u08 error)
{
  pc->status &= ~(PAMELA_STATUS_STATE_MASK | PAMELA_STATUS_ERROR_MASK);
  pc->status |= PAMELA_STATUS_OPEN_ERROR;
  pc->status |= error << PAMELA_STATUS_ERROR_SHIFT;
}

// ----- read -----

static void pamela_read_reply(pamela_channel_t *pc)
{
  // size is not the requested size
  if(pc->rx_org_size != pc->rx_size) {
    pc->status |= PAMELA_STATUS_READ_SIZE;
  }

  // read is now pending
  pc->status |= PAMELA_STATUS_READ_READY;
  pc->status &= ~PAMELA_STATUS_READ_REQ;

  DS("[rp:"); DB(pc->chan_id); DC('='); DW(pc->rx_size); DC(']');

  // notify host
  proto_io_event_mask_add_chn(pc->chan_id);
}

static void pamela_read_error(pamela_channel_t *pc)
{
  pc->status |= PAMELA_STATUS_READ_ERROR;
  pc->status &= ~PAMELA_STATUS_READ_REQ;
  pc->rx_buf = NULL;
  pc->rx_size = 0;

  DS("[RE:"); DB(pc->chan_id); DC(']'); DNL;

  // notify host
  proto_io_event_mask_add_chn(pc->chan_id);
}

void pamela_read_work(pamela_channel_t *chn,
  hnd_read_request_func_t read_req_func)
{
  DS("[rw:");

  // pass call to handler
  u08 result = PAMELA_ERROR;
  if(read_req_func != NULL) {
    result = read_req_func(chn->slot_id, &chn->rx_buf, &chn->rx_size);
  }
  DB(result);

  if(result == PAMELA_OK) {
    pamela_read_reply(chn);
  } else if(result != PAMELA_BUSY) {
    pamela_read_error(chn);
  }

  DC(']');
}

// ----- write -----

static void pamela_write_reply(pamela_channel_t *pc)
{
  // size is not the requested size
  if(pc->tx_org_size != pc->tx_size) {
    pc->status |= PAMELA_STATUS_WRITE_SIZE;
  }

  // write is now pending
  pc->status |= PAMELA_STATUS_WRITE_READY;
  pc->status &= ~PAMELA_STATUS_WRITE_REQ;

  DS("[wp:"); DB(pc->chan_id); DC('='); DW(pc->tx_size); DC(']');

  // notify host
  proto_io_event_mask_add_chn(pc->chan_id);
}

static void pamela_write_error(pamela_channel_t *pc)
{
  pc->status |= PAMELA_STATUS_WRITE_ERROR;
  pc->status &= ~PAMELA_STATUS_WRITE_REQ;
  pc->tx_buf = NULL;
  pc->tx_size = 0;

  DS("[WE:"); DB(pc->chan_id); DC(']'); DNL;

  // notify host
  proto_io_event_mask_add_chn(pc->chan_id);
}

void pamela_write_work(pamela_channel_t *chn,
  hnd_write_request_func_t write_req_func)
{
  DS("[ww:");

  u08 result = PAMELA_ERROR;
  if(write_req_func != NULL) {
    result = write_req_func(chn->slot_id, &chn->tx_buf, &chn->tx_size);
  }
  DB(result);

  if(result == PAMELA_OK) {
    pamela_write_reply(chn);
  } else if(result != PAMELA_BUSY) {
    pamela_write_error(chn);
  }

  DC(']');
}

// ----- open -----

static void pamela_open_done(pamela_channel_t *pc, u08 error)
{
  if(error == PAMELA_OK) {
    pamela_set_status(pc, PAMELA_STATUS_ACTIVE);
  } else {
    pamela_set_error(pc, error);
  }

  // set service active
  pc->service->channels |= 1 << pc->chan_id;

  DS("[Od:"); DB(error); DC(']');

  // trigger event
  proto_io_event_mask_add_chn(pc->chan_id);
}

void pamela_open_work(pamela_channel_t *chn, hnd_open_func_t open_func)
{
  pamela_service_t *srv = chn->service;

  DS("[Ow:"); DB(chn->chan_id); DC(',');
  if(open_func != NULL) {
    pamela_set_status(chn, PAMELA_STATUS_OPENING);
    u08 res = open_func(chn->slot_id, chn->chan_id, chn->port);
    // if not PAMELA_BUSY then open is done
    if((res == PAMELA_OK) || (res != PAMELA_BUSY)) {
      pamela_open_done(chn, res);
    }
  } else {
    // quick open
    pamela_open_done(chn, PAMELA_OK);
  }
  DC(']');
}

// ----- close -----

static void pamela_close_done(pamela_channel_t *pc)
{
  pamela_set_status(pc, PAMELA_STATUS_INACTIVE);

  // clear channel
  pc->service->channels &= ~(1 << pc->chan_id);

  pc->status = 0;
  pc->service = NULL;
  pc->port = 0;
  pc->slot_id = PAMELA_EMPTY_SLOT;

  DS("[Cd]");

  // trigger event
  proto_io_event_mask_add_chn(pc->chan_id);
}

void pamela_close_work(pamela_channel_t *chn, hnd_close_func_t close_func)
{
  DS("[Cw:"); DB(chn->chan_id); DC(',');
  if(close_func != NULL) {
    pamela_set_status(chn, PAMELA_STATUS_CLOSING);
    u08 res = close_func(chn->slot_id);
    // if not PAMELA_BUSY then close is done
    if(res == PAMELA_OK) {
      pamela_close_done(chn);
    }
  } else {
    // quick close without handler func
    pamela_close_done(chn);
  }
  DC(']');
}

// ----- reset -----

static void pamela_reset_done(pamela_channel_t *pc)
{
  pamela_set_status(pc, PAMELA_STATUS_ACTIVE);

  DS("[Rd]");

  // trigger event
  proto_io_event_mask_add_chn(pc->chan_id);
}

void pamela_reset_work(pamela_channel_t *chn, hnd_reset_func_t reset_func)
{
  DS("[Rw:"); DB(chn->chan_id); DC(',');
  if(reset_func != NULL) {
    pamela_set_status(chn, PAMELA_STATUS_RESETTING);
    u08 res = reset_func(chn->slot_id);
    // if not PAMELA_BUSY then close is done
    if(res == PAMELA_OK) {
      pamela_reset_done(chn);
    }
  } else {
    pamela_reset_done(chn);
  }
  DC(']');
}

// ----- channel work -----

static void pamela_active_work(pamela_channel_t *chn)
{
  //DS("{act:"); DB(chn->chan_id); DC(':'); DW(chn->status); DC('}');

  if(chn->status & PAMELA_STATUS_READ_REQ) {
    hnd_read_request_func_t read_req_func = HANDLER_FUNC_READ_WORK(chn->service->handler);
    pamela_read_work(chn, read_req_func);
  }
  if(chn->status & PAMELA_STATUS_WRITE_REQ) {
    hnd_write_request_func_t write_req_func = HANDLER_FUNC_WRITE_WORK(chn->service->handler);
    pamela_write_work(chn, write_req_func);
  }
}

static void pamela_channel_work(pamela_channel_t *chn)
{
  pamela_service_t *srv = chn->service;

  switch(chn->status & PAMELA_STATUS_STATE_MASK) {
    case PAMELA_STATUS_OPENING:
      {
        hnd_open_func_t open_func = HANDLER_FUNC_OPEN_WORK(srv->handler);
        pamela_open_work(chn, open_func);
      }
      break;
    case PAMELA_STATUS_CLOSING:
      {
        hnd_close_func_t close_func = HANDLER_FUNC_CLOSE_WORK(srv->handler);
        pamela_close_work(chn, close_func);
      }
      break;
    case PAMELA_STATUS_RESETTING:
      {
        hnd_reset_func_t reset_func = HANDLER_FUNC_RESET_WORK(srv->handler);
        pamela_reset_work(chn, reset_func);
      }
      break;
    case PAMELA_STATUS_ACTIVE:
      pamela_active_work(chn);
      break;
    default:
      break;
  }
}

// ----- API functions  -----

void pamela_end_stream(u08 chn, u08 error)
{
  pamela_channel_t *pc = pamela_get_channel(chn);

  if(error == PAMELA_OK) {
    pamela_set_status(pc, PAMELA_STATUS_EOS);
  } else {
    pamela_set_error(pc, error);
  }

  DS("[eos:"); DB(chn); DC('='); DB(error); DC(']'); DNL;

  // notify host
  proto_io_event_mask_add_chn(chn);
}
