#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "proto_shared.h"
#include "proto.h"
#include "reg.h"
#include "status.h"

void status_init(status_data_t *data)
{
  data->pending_channel = STATUS_NO_CHANNEL;
  data->event_mask = STATUS_NO_EVENTS;
  data->flags = STATUS_FLAGS_NONE;
  data->last_res = PROTO_RET_OK;
  data->last_state = 0xff;
}

int status_update(proto_handle_t *ph, status_data_t *data)
{
  // get state byte
  UBYTE state = proto_get_status(ph);

#if 0
  // anything changed?
  if(state == data->last_state) {
    return FALSE;
  }
#endif
  data->last_state = state;

  // decode state byte
  // a read is pending
  if((state & PROTO_STATUS_READ_PENDING) == PROTO_STATUS_READ_PENDING) {
    data->pending_channel = (state & PROTO_STATUS_CHANNEL_MASK) >> 4;
    data->event_mask = STATUS_NO_EVENTS;
    data->flags &= ~STATUS_FLAGS_EVENTS;
    data->flags |= STATUS_FLAGS_PENDING;
  } else {
    data->flags = STATUS_FLAGS_NONE;
    data->pending_channel = STATUS_NO_CHANNEL;
    data->event_mask = STATUS_NO_EVENTS;
    if(state & PROTO_STATUS_BUSY) {
      data->flags |= STATUS_FLAGS_BUSY;
    }
    if(state & PROTO_STATUS_ATTACHED) {
      data->flags |= STATUS_FLAGS_ATTACHED;
    }
    if(state & PROTO_STATUS_EVENTS) {
      data->flags |= STATUS_FLAGS_EVENTS;

      // try to read event mask
      UWORD e;
      data->last_res = reg_base_get_event_mask(ph, &e);
      if(data->last_res == PROTO_RET_OK) {
        data->event_mask = (UBYTE)e;
      } else {
        data->flags |= STATUS_FLAGS_NO_MASK;
        data->event_mask = 0;
      }
    }
  }
  return TRUE;
}
