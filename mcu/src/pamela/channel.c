#include "types.h"
#include "arch.h"
#include "autoconf.h"

#include "channel.h"
#include "handler.h"

/* define the channel map */
channel_t channel_map[PROTO_MAX_CHANNEL];

void channel_all_init(void)
{
  for(int i=0;i<PROTO_MAX_CHANNEL;i++) {
    channel_ptr_t chn = &channel_map[i];
    chn->status = CHANNEL_STATUS_NO_ERROR;
    chn->error_code = CHANNEL_ERROR_NONE;
    chn->mtu = 0;
    chn->handler = NULL;
  }
}

void channel_all_work(void)
{
  for(int i=0;i<PROTO_MAX_CHANNEL;i++) {
    channel_ptr_t chn = &channel_map[i];
    handler_ptr_t hnd = chn->handler;
    if(hnd != NULL) {
      hnd_work_func_t func = HANDLER_FUNC_WORK(hnd);
      if(func != NULL) {
        u16 prop = HANDLER_GET_PROPERTIES(hnd);
        if( ((prop & HANDLER_PROP_WORK_CLOSED)==0) ||
            ((chn->status & CHANNEL_STATUS_OPENED)==CHANNEL_STATUS_OPENED)) {
          func(i);
            }
      }
    }
  }
}

u08 channel_attach(u08 id, handler_ptr_t handler)
{
    channel_ptr_t chn = &channel_map[id];

    // already a handler?
    if(chn->handler != NULL) {
      return CHANNEL_ERROR_DUPL_HANDLER;
    }

    // already has an error?
    if((chn->status & CHANNEL_STATUS_NO_ERROR) == 0) {
      return CHANNEL_ERROR_HAS_ERROR;
    }

    // setup channel
    chn->handler = handler;
    chn->status |= CHANNEL_STATUS_ATTACHED;
    chn->mtu = HANDLER_GET_DEF_MTU(handler);

    // call handler
    hnd_attach_func_t func = HANDLER_FUNC_ATTACH(handler);
    if(func != NULL) {
      return func(id);
    }

    return CHANNEL_ERROR_NONE;
}

u08 channel_detach(u08 id)
{
    channel_ptr_t chn = &channel_map[id];
    handler_ptr_t handler = chn->handler;

    // already a handler?
    if(chn->handler == NULL) {
      return CHANNEL_ERROR_NOT_ATTACHED;
    }

    // already has an error?
    if((chn->status & CHANNEL_STATUS_NO_ERROR) == 0) {
      return CHANNEL_ERROR_HAS_ERROR;
    }

    // update status
    chn->status &= ~CHANNEL_STATUS_ATTACHED;
 
    // call handler
    hnd_detach_func_t func = HANDLER_FUNC_DETACH(handler);
    if(func != NULL) {
      return func(id);
    }

    return CHANNEL_ERROR_NONE;
}

void channel_open(u08 id)
{
    channel_ptr_t chn = &channel_map[id];
    handler_ptr_t handler = chn->handler;

    // not attached?
    if(chn->handler == NULL) {
      channel_set_error_code(id, CHANNEL_ERROR_NOT_ATTACHED);
      return;
    }

    // already has an error?
    if((chn->status & CHANNEL_STATUS_NO_ERROR) == 0) {
      channel_set_error_code(id, CHANNEL_ERROR_HAS_ERROR);
      return;
    }

    // update status
    chn->status |= CHANNEL_STATUS_OPENED;
 
    // call handler
    hnd_open_func_t func = HANDLER_FUNC_OPEN(handler);
    if(func != NULL) {
      func(id);
    }
}

void channel_close(u08 id)
{

}

void channel_reset(u08 id)
{

}

extern void channel_set_mtu(u08 id, u16 mtu);
extern u16  channel_get_properties(u08 id);
extern u16  channel_get_def_mtu(u08 id);

/* called by handler code */
extern void channel_set_error_code(u08 id, u16 error_code);
extern void channel_send(u08 id, u16 size, u08 *buf);



u16 channel_rx_get_size(u08 chan)
{
  if(chn->status & CHANNEL_STATUS_RX_PENDING) {
    status_clr_rx_flag(chan);
    chn->status &= ~CHANNEL_STATUS_RX_PENDING;
    chn->status |= CHANNEL_STATUS_RX_OP;
    return chn->msg_size;
  } else {
    channel_set_error(chan, CHANNEL_ERROR_RX_EMPTY);
  }
}