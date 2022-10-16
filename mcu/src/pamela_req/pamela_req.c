#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_PAMELA_REQ

#include "debug.h"

#include "pamela_req.h"
#include "pamela_req_int.h"

struct pamela_req_service {
  pamela_req_handler_ptr_t  req_handler;
  pamela_handler_ptr_t      pam_handler;
  pamela_req_slot_t *slots;
  u08 srv_id;
};
typedef struct pamela_req_service pamela_req_service_t;


// global service list
static pamela_req_service_t pamela_req_services[PAMELA_REQ_NUM_HANDLERS];
// number of currently added services
static u08 num_services;


u08 pamela_req_add_handler(pamela_handler_ptr_t pam_hnd,
                           pamela_req_handler_ptr_t req_hnd,
                           pamela_req_slot_t *slots)
{
  if(num_services == PAMELA_REQ_NUM_HANDLERS) {
    return PAMELA_REQ_NO_SERVICE_ID;
  }

  /* add a regular pamela service */
  u08 srv_id = pamela_add_handler(pam_hnd);
  if(srv_id == PAMELA_NO_SERVICE_ID) {
    return PAMELA_REQ_NO_SERVICE_ID;
  }

  /* register a req service */
  pamela_req_service_t *srv = &pamela_req_services[num_services];
  srv->srv_id = srv_id;
  srv->pam_handler = pam_hnd;
  srv->req_handler = req_hnd;
  srv->slots = slots;

  /* get next id */
  u08 cid = num_services;
  num_services++;
  return cid;
}

static pamela_req_service_t *find_service(u08 chan)
{
  u08 srv_id = pamela_get_srv_id(chan);
  /* find the req service */
  pamela_req_service_t *ptr = &pamela_req_services[0];
  for(u08 i=0;i<num_services;i++) {
    if(ptr->srv_id == srv_id) {
      return ptr;
    }
    ptr++;
  }
  DS("ERROR: can't find req service!!"); DNL;
  return NULL;
}

static pamela_req_slot_t *find_slot(u08 chan)
{
  pamela_req_service_t *srv = find_service(chan);
  if(srv == NULL) {
    return NULL;
  }

  u08 slot = pamela_get_slot(chan);
  u08 num_slots = HANDLER_GET_MAX_SLOTS(srv->pam_handler);
  if(slot >= num_slots) {
    DS("ERROR: can't find req slot: "); DB(slot); DNL;
    return NULL;
  }

  return &srv->slots[slot];
}

// ----- pamela req service -----

u08 pamela_req_open(u08 chan, u16 port)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return PAMELA_ERROR;
  }

  DS("#req: open "); DB(chan); DC(':'); DW(port); DNL;

  /* init state of slot */
  slot->state = PAMELA_REQ_STATE_IDLE;

  return PAMELA_OK;
}

u08 pamela_req_close(u08 chan)
{
  DS("#req: close "); DB(chan); DNL;

  /* nop */

  return PAMELA_OK;
}

u08 pamela_req_reset(u08 chan)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return PAMELA_ERROR;
  }

  DS("#req: reset: "); DB(chan); DNL;

  /* reset state of slot */
  /* TODO: cleanup pending req */
  slot->state = PAMELA_REQ_STATE_IDLE;

  return PAMELA_OK;
}

// ----- write -----

u08 pamela_req_write_request(u08 chan, u08 **buf, u16 *size)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return PAMELA_ERROR;
  }

  DS("#req: wr: "); DB(chan);

  // check state: must be idle
  if(slot->state != PAMELA_REQ_STATE_IDLE) {
    DS(" wrong state!"); DNL;
    return PAMELA_ERROR;
  }

  // call req handler begin()
  pamela_req_service_t *srv = find_service(chan);
  hnd_req_begin_func_t begin_func = REQ_HANDLER_FUNC_BEGIN(srv->req_handler);
  u08 result = begin_func(chan, buf, *size);
  DS("begin: "); DW(*size); DC('^'); DB(result);
  if(result != PAMELA_OK) {
    // error needs reset of channel
    DS(" failed!"); DNL;
    return result;
  }

  // enter state IN_WRITE
  slot->state = PAMELA_REQ_STATE_IN_WRITE;

  DNL;
  return PAMELA_OK;
}

void pamela_req_write_done(u08 chan, u08 *buf, u16 size)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return;
  }

  DS("#req: wd: "); DB(chan);

  // check state: must be in write
  if(slot->state != PAMELA_REQ_STATE_IN_WRITE) {
    DS(" wrong state!"); DNL;
    return;
  }

  // call req handler handle()
  pamela_req_service_t *srv = find_service(chan);
  hnd_req_handle_func_t handle_func = REQ_HANDLER_FUNC_HANDLE(srv->req_handler);
  u08 result = handle_func(chan, buf, size, &slot->reply_buf, &slot->reply_size);
  DS("handle: "); DW(size); DS(" -> "); DW(slot->reply_size); DNL;

  if(result == PAMELA_OK) {
    slot->state = PAMELA_REQ_STATE_READ_WAIT;
  } else if(result == PAMELA_ERROR) {
    // error needs reset of channel
    pamela_end_stream(chan, PAMELA_ERROR);
  } else {
    // TODO: start worker for handle_work()
    slot->state = PAMELA_REQ_STATE_BUSY;
  }
}

// ----- read -----

u08 pamela_req_read_request(u08 chan, u08 **buf, u16 *size)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return PAMELA_ERROR;
  }

  DS("#req: rr: "); DB(chan);

  // check state: must be READ_WAIT
  if(slot->state != PAMELA_REQ_STATE_READ_WAIT) {
    DS(" wrong state!"); DNL;
    return PAMELA_ERROR;
  }

  // enter state IN_READ
  slot->state = PAMELA_REQ_STATE_IN_READ;

  DS(" size:"); DB(slot->reply_size); DNL;

  *buf = slot->reply_buf;
  *size = slot->reply_size;

  return PAMELA_OK;
}

void pamela_req_read_done(u08 chan, u08 *buf, u16 size)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return;
  }

  DS("#req: rd: "); DB(chan); DNL;

  // call handler end()
  pamela_req_service_t *srv = find_service(chan);
  hnd_req_end_func_t end_func = REQ_HANDLER_FUNC_END(srv->req_handler);
  end_func(chan, slot->reply_buf, slot->reply_size);
  DS("end: "); DW(slot->reply_size); DNL;

  slot->state = PAMELA_REQ_STATE_IDLE;
}

