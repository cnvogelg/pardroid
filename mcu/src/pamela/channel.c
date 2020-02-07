#include "types.h"
#include "arch.h"
#include "autoconf.h"

#include "channel.h"
#include "handler.h"
#include "status.h"

static channel_ptr_t get_channel(u08 id)
{
  channel_ptr_t chn = channel_api_get_channel(id);
  if(chn != NULL) {
    // not attached?
    if(chn->handler == NULL) {
      channel_set_error_code(id, CHANNEL_ERROR_ATTACH);
      return NULL;
    }
    return chn;
  } else {
    channel_set_error_code(id, CHANNEL_ERROR_HANDLER_OPEN);
    return NULL;
  }
}

// ----- direct user API -----

void channel_init(channel_ptr_t chn, handler_ptr_t handler)
{
  if(chn != NULL) {
    chn->state = CHANNEL_STATE_ATTACHED;
    chn->error_code = CHANNEL_ERROR_NONE;
    chn->mtu = 0;
    chn->handler = handler;
    chn->mtu = HANDLER_GET_DEF_MTU(handler);
  }
}

void channel_work(u08 id)
{
  channel_ptr_t chn = channel_api_get_channel(id);
  if(chn != NULL) {
    handler_ptr_t hnd = chn->handler;
    if(hnd != NULL) {
      hnd_work_func_t func = HANDLER_FUNC_WORK(hnd);
      if(func != NULL) {
        if((chn->state & CHANNEL_STATE_OPENED)==CHANNEL_STATE_OPENED) {
          func(id);
        }
      }
    }
  }
}

void channel_work_all()
{
  u16 mask = channel_api_get_mask();
  u16 m = 1;
  for(u08 i=0;i<PROTO_MAX_CHANNEL;i++) {
    if((mask & m) == m) {
      channel_work(i);
    }
    m <<= 1;
  }
}

// ----- Proto API -----

void channel_open(u08 id)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return;
  }
  handler_ptr_t handler = chn->handler;

  // update status
  chn->state |= CHANNEL_STATE_OPENED;
  chn->tr_num_words = 0;
  chn->tr_got_words = 0;
  chn->tr_offset = 0;

  // call handler
  hnd_open_func_t func = HANDLER_FUNC_OPEN(handler);
  if(func != NULL) {
    u08 result = func(id);
    if(result != 0) {
      channel_set_error_code(id, CHANNEL_ERROR_HANDLER_OPEN);
    }
  }
}

void channel_close(u08 id)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return;
  }
  handler_ptr_t handler = chn->handler;

  // update status
  chn->state &= ~CHANNEL_STATE_OPENED;

  // call handler
  hnd_close_func_t func = HANDLER_FUNC_CLOSE(handler);
  if(func != NULL) {
    func(id);
  }
}

void channel_reset(u08 id)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return;
  }
  handler_ptr_t handler = chn->handler;

  // reset status
  chn->state = CHANNEL_STATE_ATTACHED;

  // call handler
  hnd_reset_func_t func = HANDLER_FUNC_RESET(handler);
  if(func != NULL) {
    func(id);
  }
}

void channel_set_mtu(u08 id, u16 mtu)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return;
  }
  handler_ptr_t handler = chn->handler;

  // call handler
  hnd_set_mtu_func_t func = HANDLER_FUNC_SET_MTU(handler);
  if(func != NULL) {
    chn->mtu = func(id, mtu);
  }
}

u16 channel_get_mode(u08 id)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return 0;
  }
  handler_ptr_t handler = chn->handler;

  return HANDLER_GET_MODE(handler);
}

u16 channel_get_def_mtu(u08 id)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return 0;
  }
  handler_ptr_t handler = chn->handler;

  return HANDLER_GET_DEF_MTU(handler);
}

u16 channel_get_mtu(u08 id)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return 0;
  }
  return chn->mtu;
}

void channel_set_offset(u08 id, u32 offset)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return;
  }
  chn->tr_offset = offset;
}

u32 channel_get_offset(u08 id)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return 0;
  }
  return chn->tr_offset;
}

u16 channel_get_error_code(u08 id)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return 0;
  }
  u16 code = chn->error_code;
  status_clr_error_flag(id);
  chn->error_code = 0;
  chn->state &= ~CHANNEL_STATE_ERROR;
  return code;
}

/* called by handler code */
void channel_set_error_code(u08 id, u16 error_code)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return;
  }
  chn->error_code = error_code;
  chn->state |= CHANNEL_STATE_ERROR;
  status_set_error_flag(id);
}

/* read op */
u16 channel_read_begin(u08 id)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return 0;
  }
  handler_ptr_t handler = chn->handler;

  // is a transfer operation running?
  if(chn->state & CHANNEL_STATE_OP_MASK) {
    channel_set_error_code(id, CHANNEL_ERROR_OP_RUNNING);
    return 0;
  }

  // ask handler for size of read
  hnd_read_begin_func_t func = HANDLER_FUNC_READ_BEGIN(handler);
  u16 num_words = 0;
  if(func != NULL) {
    num_words = func(id, chn->mtu, chn->tr_offset);
  }

  // setup transfer state
  chn->tr_num_words = num_words;
  chn->tr_got_words = 0;
  chn->state |= CHANNEL_STATE_RX_OP;
  return num_words;
}

u08 *channel_read_chunk_begin(u08 id, u16 *ret_size)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return NULL;
  }
  handler_ptr_t handler = chn->handler;

  // is a transfer operation running?
  if((chn->state & CHANNEL_STATE_OP_MASK) != CHANNEL_STATE_RX_OP) {
    channel_set_error_code(id, CHANNEL_ERROR_OP_RUNNING);
    return NULL;
  }

  // calc size of chunk
  u16 size = chn->tr_num_words - chn->tr_got_words;
  if(size > chn->mtu) {
    size = chn->mtu;
  }
  *ret_size = size;

  // get filled buffer pointer from handler
  u08 *buf = NULL;
  hnd_read_chunk_begin_func_t func = HANDLER_FUNC_READ_CHUNK_BEGIN(handler);
  if(func != NULL) {
    buf = func(id, chn->tr_got_words, size);
  }

  return buf;
}

void channel_read_chunk_end(u08 id)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return;
  }
  handler_ptr_t handler = chn->handler;

  // is a transfer operation running?
  if((chn->state & CHANNEL_STATE_OP_MASK) != CHANNEL_STATE_RX_OP) {
    channel_set_error_code(id, CHANNEL_ERROR_OP_RUNNING);
    return;
  }

  // report back
  hnd_read_chunk_end_func_t func = HANDLER_FUNC_READ_CHUNK_END(handler);
  if(func != NULL) {
    func(id);
  }

  // read finished?
  if(chn->tr_got_words == chn->tr_num_words) {
    hnd_read_end_func_t func = HANDLER_FUNC_READ_END(handler);
    if(func != NULL) {
      func(id, 0);
    }
  }
}

/* write op */
void channel_write_begin(u08 id, u16 num_words)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return;
  }
  handler_ptr_t handler = chn->handler;

  // is a transfer operation running?
  if(chn->state & CHANNEL_STATE_OP_MASK) {
    channel_set_error_code(id, CHANNEL_ERROR_OP_RUNNING);
    return;
  }

  // tell handler size of write
  hnd_write_begin_func_t func = HANDLER_FUNC_WRITE_BEGIN(handler);
  if(func != NULL) {
    func(id, chn->mtu, chn->tr_offset, num_words);
  }

  // setup transfer state
  chn->tr_num_words = num_words;
  chn->tr_got_words = 0;
  chn->state |= CHANNEL_STATE_TX_OP;
}

u08 *channel_write_chunk_begin(u08 id, u16 *ret_size)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return NULL;
  }
  handler_ptr_t handler = chn->handler;

  // is a transfer operation running?
  if((chn->state & CHANNEL_STATE_OP_MASK) != CHANNEL_STATE_TX_OP) {
    channel_set_error_code(id, CHANNEL_ERROR_OP_RUNNING);
    return NULL;
  }

  // calc size of chunk
  u16 size = chn->tr_num_words - chn->tr_got_words;
  if(size > chn->mtu) {
    size = chn->mtu;
  }
  *ret_size = size;

  // get buffer from handler
  u08 *buf = NULL;
  hnd_write_chunk_begin_func_t func = HANDLER_FUNC_WRITE_CHUNK_BEGIN(handler);
  if(func != NULL) {
    buf = func(id, chn->tr_got_words, size);
  }

  return buf;
}

void channel_write_chunk_end(u08 id)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return;
  }
  handler_ptr_t handler = chn->handler;

  // is a transfer operation running?
  if((chn->state & CHANNEL_STATE_OP_MASK) != CHANNEL_STATE_TX_OP) {
    channel_set_error_code(id, CHANNEL_ERROR_OP_RUNNING);
    return;
  }

  // report back
  hnd_write_chunk_end_func_t func = HANDLER_FUNC_WRITE_CHUNK_END(handler);
  if(func != NULL) {
    func(id);
  }

  // read finished?
  if(chn->tr_got_words == chn->tr_num_words) {
    hnd_write_end_func_t func = HANDLER_FUNC_WRITE_END(handler);
    if(func != NULL) {
      func(id, 0);
    }
  }
}

/* cancel transfer */
void channel_transfer_cancel(u08 id)
{
  channel_ptr_t chn = get_channel(id);
  if(chn == NULL) {
    return;
  }
  handler_ptr_t handler = chn->handler;

  // abort read
  u08 state = chn->state;
  if((state & CHANNEL_STATE_RX_OP)==CHANNEL_STATE_RX_OP) {
    hnd_read_end_func_t func = HANDLER_FUNC_READ_END(handler);
    if(func != NULL) {
      func(id, 1);
    }
    chn->state &= ~CHANNEL_STATE_RX_OP;
  }
  // abort write
  else if((state & CHANNEL_STATE_TX_OP)==CHANNEL_STATE_TX_OP) {
    hnd_write_end_func_t func = HANDLER_FUNC_WRITE_END(handler);
    if(func != NULL) {
      func(id, 1);
    }
    chn->state &= ~CHANNEL_STATE_TX_OP;
  }
  else {
    channel_set_error_code(id, CHANNEL_ERROR_OP_RUNNING);
  }
}

// proto API binding
#define BIND(x) __attribute__ ((weak, alias(#x)))

void proto_api_chn_set_offset(u08 chn, u32 offset)  BIND(channel_set_offset);
u16  proto_api_chn_get_rx_size(u08 chan)            BIND(channel_read_begin);
void proto_api_chn_set_tx_size(u08 chan, u16 size)  BIND(channel_write_begin);
void proto_api_chn_cancel_transfer(u08 chan)        BIND(channel_transfer_cancel);

u08 *proto_api_read_msg_begin(u08 chan, u16 *ret_size)  BIND(channel_read_chunk_begin);
void proto_api_read_msg_done(u08 chan)                  BIND(channel_read_chunk_end);
u08 *proto_api_write_msg_begin(u08 chan, u16 *ret_size) BIND(channel_write_chunk_begin);
void proto_api_write_msg_done(u08 chan)                 BIND(channel_write_chunk_end);
