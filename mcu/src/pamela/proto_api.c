/* implement the proto_api_* and map to channel handler */

#include "types.h"
#include "autoconf.h"
#include "arch.h"

#define DEBUG CONFIG_DEBUG_PROTO

#include "proto_api.h"
#include "channel.h"
#include "fw_info.h"
#include "machtag.h"
#include "status.h"

/* keep a channel index for channel-related global commands */
static u08 channel_id;

void proto_api_action(u08 num)
{
  /* handle channel actions */
  switch (num) {
    case PROTO_ACTION_CHN_OPEN:
      channel_open(channel_id);
      break;
    case PROTO_ACTION_CHN_CLOSE:
      channel_close(channel_id);
      break;
    case PROTO_ACTION_CHN_RESET:
      channel_reset(channel_id);
      break;
  }
}

u16 proto_api_wfunc_read(u08 num)
{
  switch (num) {
    /* firmware info */
    case PROTO_WFUNC_READ_FW_ID:
      return FW_GET_ID();
    case PROTO_WFUNC_READ_FW_VERSION:
      return FW_GET_VERSION();
    case PROTO_WFUNC_READ_MACHTAG:
      return FW_GET_MACHTAG();
    /* read status/error mask */
    case PROTO_WFUNC_READ_STATUS_MASK:
      return status_get_status_mask();
    case PROTO_WFUNC_READ_ERROR_MASK:
      return status_get_error_mask();
    /* channel functions */
    case PROTO_WFUNC_READ_CHN_INDEX:
      return channel_id;
    case PROTO_WFUNC_READ_CHN_MTU:
      return channel_map[channel_id].mtu;
    case PROTO_WFUNC_READ_CHN_ERROR_CODE:
      return channel_map[channel_id].error_code;
    case PROTO_WFUNC_READ_CHN_PROPERTIES:
      return channel_get_properties(channel_id);
    case PROTO_WFUNC_READ_CHN_DEF_MTU:
      return channel_get_def_mtu(channel_id);
    default:
      return 0;
  }
}

void proto_api_wfunc_write(u08 num, u16 val)
{
  switch(num) {
    case PROTO_WFUNC_WRITE_CHN_INDEX:
      {
        if(val < PROTO_MAX_CHANNEL) {
          channel_id = val;
        }
      }
      break;
    case PROTO_WFUNC_WRITE_CHN_MTU:
      channel_set_mtu(channel_id, val);
      break;
    default:
      break;
  }
}

u32 proto_api_lfunc_read(u08 num)
{
  switch(num) {
    case PROTO_LFUNC_READ_CHN_RX_OFFSET:
      return channel_map[channel_id].rx_offset;
    case PROTO_LFUNC_READ_CHN_TX_OFFSET:
      return channel_map[channel_id].tx_offset;
    default:
      return 0;
  }
}

void proto_api_lfunc_write(u08 num, u32 val)
{
}

// channel ops
void proto_api_chn_set_rx_offset(u08 chan, u32 offset)
{
  channel_map[chan].rx_offset = offset;
}

void proto_api_chn_set_tx_offset(u08 chan, u32 offset)
{
  channel_map[chan].tx_offset = offset;
}

u16  proto_api_chn_get_rx_size(u08 chan)
{
  return channel_map[chan].rx_size;
}

void proto_api_chn_set_rx_size(u08 chan, u16 size)
{
  channel_map[chan].max_rx_size = size;
}

void proto_api_chn_set_tx_size(u08 chan, u16 size)
{
  channel_map[chan].tx_size = size;
}

void proto_api_chn_request_rx(u08 chan)
{
  channel_ptr_t channel = &channel_map[chan];
  
  // is sane?
  u16 status = channel->status;
  if((status & CHANNEL_MASK_VALID) != CHANNEL_MASK_VALID) {
    return;
  }

  // already a request pending?
  if(status & CHANNEL_STATUS_RX_REQUEST) {
    channel_set_error_code(chan, CHANNEL_ERROR_DUPL_RX_REQ);
    return;
  }

  // init rx state
  channel->status |= CHANNEL_STATUS_RX_REQUEST;

  // call handler func
  handler_ptr_t hnd = channel->handler;
  hnd_rx_request_func_t func = HANDLER_FUNC_RX_REQUEST(hnd);
  if(func != NULL) {
    func(chan);
  }
}

void proto_api_chn_cancel_rx(u08 chan)
{

}

void proto_api_chn_cancel_tx(u08 chan)
{
  
}

// message read

u08 *proto_api_read_msg_begin(u08 chan, u16 *ret_size)
{
  channel_ptr_t channel = &channel_map[chan];

  // is sane to start transfer
  u16 status = channel->status;
  if((status & CHANNEL_MASK_VALID) != CHANNEL_MASK_VALID) {
    return NULL;
  }

  // begin of transfer
  if(channel->rx_frag_off == 0) {
    status |= CHANNEL_STATUS_RX_OP;
  }

  // calc msg size
  u16 mtu = channel->mtu;
  u16 size = channel->rx_size - channel->rx_frag_off;
  if(size > mtu) {
    size = mtu;
  }
  *ret_size = size;

  // call handler func
  handler_ptr_t hnd = channel->handler;
  hnd_read_begin_func_t func = HANDLER_FUNC_READ_BEGIN(hnd);
  if(func != NULL) {
    func(chan, size, channel->rx_frag_off, channel->rx_size);
  }

  // update fragment offset
  channel->rx_frag_off += size;

  return channel->rx_buf;
}

void proto_api_read_msg_done(u08 chan)
{
  channel_ptr_t channel = &channel_map[chan];

  // call handler func
  handler_ptr_t hnd = channel->handler;
  hnd_read_end_func_t func = HANDLER_FUNC_READ_END(hnd);
  if(func != NULL) {
    func(chan);
  }

  // done with transfer?
  if(channel->rx_frag_off == channel->rx_size) {
    // done with transfer
    channel->status &= ~(CHANNEL_STATUS_RX_OP | CHANNEL_STATUS_RX_REQUEST);
    channel->rx_frag_off = 0;
  }
}

// message write

u08 *proto_api_write_msg_begin(u08 chan, u16 *ret_size)
{
  channel_ptr_t channel = &channel_map[chan];

  // is sane to start transfer
  u16 status = channel->status;
  if((status & CHANNEL_MASK_VALID) != CHANNEL_MASK_VALID) {
    return NULL;
  }

  // begin of transfer
  if(channel->tx_frag_off == 0) {
    status |= CHANNEL_STATUS_TX_OP;
  }

  // calc msg size
  u16 mtu = channel->mtu;
  u16 size = channel->tx_size - channel->tx_frag_off;
  if(size > mtu) {
    size = mtu;
  }
  *ret_size = size;

  // call handler func
  handler_ptr_t hnd = channel->handler;
  hnd_write_begin_func_t func = HANDLER_FUNC_WRITE_BEGIN(hnd);
  u08 *buf = NULL;
  if(func != NULL) {
    buf = func(chan, size, channel->tx_frag_off, channel->rx_size);
  }

  // update fragment offset
  channel->tx_frag_off += size;

  return buf;
}

void proto_api_write_msg_done(u08 chan)
{
  channel_ptr_t channel = &channel_map[chan];

  // call handler func
  handler_ptr_t hnd = channel->handler;
  hnd_read_end_func_t func = HANDLER_FUNC_READ_END(hnd);
  if(func != NULL) {
    func(chan);
  }

  // done with transfer?
  if(channel->tx_frag_off == channel->tx_size) {
    // done with transfer
    channel->status &= ~CHANNEL_STATUS_TX_OP;
    channel->tx_frag_off = 0;
  }
}
