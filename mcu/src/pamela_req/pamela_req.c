#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_PAMELA_REQ

#include "debug.h"

#include "pamela_req.h"
#include "pamela_req_int.h"

REQ_HANDLER_TABLE_DECLARE

static pamela_req_handler_ptr_t find_req_handler(u08 chan)
{
  pamela_handler_ptr_t pam_handler  = pamela_get_handler(chan);

  /* walk through req table and find handler */
  for(u08 i=0;i<REQ_HANDLER_TABLE_GET_SIZE();i++) {
    pamela_req_handler_ptr_t handler = REQ_HANDLER_TABLE_GET_ENTRY(i);
    pamela_handler_ptr_t this_pam_handler = REQ_HANDLER_GET_HANDLER(handler);
    if(this_pam_handler == pam_handler) {
      return handler;
    }
  }
  DS("ERROR: can't find req service!!"); DNL;
  return NULL;
}

static pamela_req_slot_t *find_slot(u08 chan)
{
  pamela_req_handler_ptr_t req_handler = find_req_handler(chan);
  if(req_handler == NULL) {
    return NULL;
  }

  u08 slot = pamela_get_slot(chan);
  pamela_handler_ptr_t pam_handler  = pamela_get_handler(chan);
  u08 num_slots = HANDLER_GET_MAX_SLOTS(pam_handler);
  if(slot >= num_slots) {
    DS("ERROR: can't find req slot: "); DB(slot); DNL;
    return NULL;
  }

  pamela_req_slot_t *slots = REQ_HANDLER_GET_SLOTS(req_handler);
  return &slots[slot];
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
  pamela_req_handler_ptr_t req_handler = find_req_handler(chan);
  hnd_req_begin_func_t begin_func = REQ_HANDLER_FUNC_BEGIN(req_handler);
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
  pamela_req_handler_ptr_t req_handler = find_req_handler(chan);
  hnd_req_handle_func_t handle_func = REQ_HANDLER_FUNC_HANDLE(req_handler);
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
  pamela_req_handler_ptr_t req_handler = find_req_handler(chan);
  hnd_req_end_func_t end_func = REQ_HANDLER_FUNC_END(req_handler);
  end_func(chan, slot->reply_buf, slot->reply_size);
  DS("end: "); DW(slot->reply_size); DNL;

  slot->state = PAMELA_REQ_STATE_IDLE;
}

