#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_PAMELA_REQ

#include "debug.h"

#include "pamela_req.h"
#include "pamela_req_int.h"

#include <stdlib.h>

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

u08 pamela_req_open(u08 chan, u08 state, u16 port)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return PAMELA_WIRE_ERROR_NO_SLOT;
  }

  DS("#req: open "); DB(chan); DC(':'); DW(port); DNL;

  /* init state of slot */
  pamela_handler_ptr_t handler = pamela_get_handler(chan);
  slot->state = PAMELA_REQ_STATE_IDLE;
  slot->global_buf.data = NULL;
  slot->global_buf.size = HANDLER_GET_MAX_MTU(handler);

  /* call req handler open */
  u08 result = PAMELA_HANDLER_OK;
  pamela_req_handler_ptr_t req_handler = find_req_handler(chan);
  hnd_req_open_func_t open_func = REQ_HANDLER_FUNC_OPEN(req_handler);
  if(open_func != NULL) {
    DS("func{");
    result = open_func(chan, &slot->global_buf);
    DC('}'); DB(result); DNL;
  }

  return result;
}

u08 pamela_req_close(u08 chan, u08 state)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return PAMELA_WIRE_ERROR_NO_SLOT;
  }

  DS("#req: close "); DB(chan); DNL;

  /* call req handler close */
  pamela_req_handler_ptr_t req_handler = find_req_handler(chan);
  hnd_req_close_func_t close_func = REQ_HANDLER_FUNC_CLOSE(req_handler);
  if(close_func != NULL) {
    DS("func{");
    close_func(chan, &slot->global_buf);
    DC('}'); DNL;
  }

  return PAMELA_HANDLER_OK;
}

u08 pamela_req_reset(u08 chan, u08 state)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return PAMELA_WIRE_ERROR_NO_SLOT;
  }

  DS("#req: reset: "); DB(chan); DNL;

  /* reset state of slot */
  /* TODO: cleanup pending req */
  slot->state = PAMELA_REQ_STATE_IDLE;

  /* call req handler reset */
  u08 result = PAMELA_HANDLER_OK;
  pamela_req_handler_ptr_t req_handler = find_req_handler(chan);
  hnd_req_reset_func_t reset_func = REQ_HANDLER_FUNC_RESET(req_handler);
  if(reset_func != NULL) {
    DS("func{");
    result = reset_func(chan);
    DC('}'); DB(result); DNL;
  }

  return PAMELA_HANDLER_OK;
}

// ----- write -----

u08 pamela_req_write_pre(u08 chan, u08 state, pamela_buf_t *buf)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return PAMELA_WIRE_ERROR_NO_SLOT;
  }

  DS("#req: wr: "); DB(chan);

  // check state: must be idle
  if(slot->state != PAMELA_REQ_STATE_IDLE) {
    DS(" wrong state!"); DNL;
    return PAMELA_WIRE_ERROR_WRONG_STATE;
  }

  // by default use global buffer (but keep requested size)
  buf->data = slot->global_buf.data;

  // call req handler begin()
  pamela_req_handler_ptr_t req_handler = find_req_handler(chan);
  hnd_req_begin_func_t begin_func = REQ_HANDLER_FUNC_BEGIN(req_handler);
  if(begin_func != NULL) {
    u08 result = begin_func(chan, buf);
    DS("begin: "); DW(buf->size); DC('^'); DB(result);
    if(result != PAMELA_HANDLER_OK) {
      // error needs reset of channel
      DS(" failed!"); DNL;
      return result;
    }
  }

  // enter state IN_WRITE
  slot->state = PAMELA_REQ_STATE_IN_WRITE;

  DNL;
  return PAMELA_HANDLER_OK;
}

u08 pamela_req_write_post(u08 chan, u08 state, pamela_buf_t *buf)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return PAMELA_WIRE_ERROR_NO_SLOT;
  }

  DS("#req: wd: "); DB(chan);

  // check state: must be in write
  if(slot->state != PAMELA_REQ_STATE_IN_WRITE) {
    DS(" wrong state!"); DNL;
    return PAMELA_WIRE_ERROR_WRONG_STATE;;
  }

  // copy (request) buf to slot (reply)
  slot->buf.data = buf->data;
  slot->buf.size = buf->size;

  // call req handler handle()
  // it will consume the request and prepare the reply
  // buf->size will be resized accordingly.
  // even buf->data could change if a new reply buffer is used
  pamela_req_handler_ptr_t req_handler = find_req_handler(chan);
  hnd_req_handle_func_t handle_func = REQ_HANDLER_FUNC_HANDLE(req_handler);
  DS("handle: "); DW(slot->buf.size);
  u08 result = handle_func(chan, PAMELA_CALL_FIRST, &slot->buf);
  if(result == PAMELA_HANDLER_OK) {
    DS(" -> "); DW(slot->buf.size); DNL;
    slot->state = PAMELA_REQ_STATE_READ_WAIT;
  } else if(result != PAMELA_HANDLER_POLL) {
    DS("ERR!"); DNL;
    return result;
  } else {
    DS("..."); DNL;
    // enable channel task for polling handler
    slot->state = PAMELA_REQ_STATE_POLLING;
    pamela_channel_task_control(chan, PAMELA_TASK_ON);
  }
  return PAMELA_HANDLER_OK;
}

// ----- read -----

u08 pamela_req_read_pre(u08 chan, u08 state, pamela_buf_t *buf)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return PAMELA_WIRE_ERROR_NO_SLOT;
  }

  DS("#req: rr: "); DB(chan);

  // check state: must be READ_WAIT
  if(slot->state != PAMELA_REQ_STATE_READ_WAIT) {
    DS(" wrong state!"); DNL;
    return PAMELA_WIRE_ERROR_WRONG_STATE;
  }

  // enter state IN_READ
  slot->state = PAMELA_REQ_STATE_IN_READ;

  DS(" size:"); DW(slot->buf.size); DNL;

  // let's read from slot's buffer
  buf->data = slot->buf.data;
  buf->size = slot->buf.size;

  return PAMELA_HANDLER_OK;
}

u08 pamela_req_read_post(u08 chan, u08 state, pamela_buf_t *buf)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return PAMELA_WIRE_ERROR_NO_SLOT;
  }

  DS("#req: rd: "); DB(chan); DNL;

  // call handler end()
  pamela_req_handler_ptr_t req_handler = find_req_handler(chan);
  hnd_req_end_func_t end_func = REQ_HANDLER_FUNC_END(req_handler);
  if(end_func != NULL) {
    end_func(chan, &slot->buf);
    DS("end: "); DW(slot->buf.size);
  }
  DNL;

  slot->state = PAMELA_REQ_STATE_IDLE;
  return PAMELA_HANDLER_OK;
}

// ----- channel task -----

void pamela_req_channel_task(u08 chan)
{
  pamela_req_slot_t *slot = find_slot(chan);
  if(slot == NULL) {
    return;
  }

  DS("#req: task: "); DB(chan);

  // check state: must be in write
  if(slot->state != PAMELA_REQ_STATE_POLLING) {
    DS(" wrong state!"); DNL;
    return;
  }

  // call req handler handle_poll()
  pamela_req_handler_ptr_t req_handler = find_req_handler(chan);
  hnd_req_handle_func_t handle_func = REQ_HANDLER_FUNC_HANDLE(req_handler);
  DS("handle_poll: "); DW(slot->buf.size);
  u08 result = handle_func(chan, PAMELA_CALL_NEXT, &slot->buf);

  if(result == PAMELA_HANDLER_OK) {
    // stop task
    DS(" -> "); DW(slot->buf.size); DNL;
    pamela_channel_task_control(chan, PAMELA_TASK_OFF);
    slot->state = PAMELA_REQ_STATE_READ_WAIT;
  } else if(result != PAMELA_HANDLER_POLL) {
    DS("ERR!"); DNL;
    // stop task
    pamela_channel_task_control(chan, PAMELA_TASK_OFF);
    // error needs reset of channel
    pamela_end_stream(chan, result);
  }
  DS("..."); DNL;
}

/* allocate the req buffer when opening a channel and free it on close */
u08 pamela_req_open_malloc(u08 chan, pamela_buf_t *buf)
{
  pamela_handler_ptr_t handler = pamela_get_handler(chan);
  u16 mtu = HANDLER_GET_MAX_MTU(handler);

  DS("#req: open/malloc:"); DW(mtu);
  buf->size = mtu;
  buf->data = (u08 *)malloc(mtu);
  if(buf->data == NULL) {
    DS("no mem!"); DNL;
    return PAMELA_WIRE_ERROR_NO_MEM;
  }
  DNL;
  return PAMELA_HANDLER_OK;
}

void pamela_req_close_free(u08 chan, pamela_buf_t *buf)
{
  DS("#req: close/free"); DW(buf->size);
  free(buf->data);
  buf->data = NULL;
  DNL;
}
