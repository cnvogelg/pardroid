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
  data->error = STATUS_NO_ERROR;
  data->flags = STATUS_FLAGS_INIT;
}

int status_update(proto_handle_t *ph, status_data_t *data)
{
  int res = PROTO_RET_OK;

  // get state byte
  UBYTE state = proto_get_status(ph);

  // decode state byte
  // a read is pending
  if((state & PROTO_STATUS_READ_PENDING) == PROTO_STATUS_READ_PENDING) {
    data->pending_channel = state & PROTO_STATUS_CHANNEL_MASK;
    data->error = STATUS_NO_ERROR;
    data->flags &= ~STATUS_FLAGS_ERROR;
    data->flags |= STATUS_FLAGS_PENDING;
  } else {
    data->flags = 0;
    data->pending_channel = STATUS_NO_CHANNEL;
    data->error = STATUS_NO_ERROR;
    if(state & PROTO_STATUS_BOOTLOADER) {
      data->flags |= STATUS_FLAGS_BOOTLOADER;
    }
    if(state & PROTO_STATUS_ATTACHED) {
      data->flags |= STATUS_FLAGS_ATTACHED;
    }
    if(state & PROTO_STATUS_ERROR) {
      data->flags |= STATUS_FLAGS_ERROR;

      // try to read error mask
      UWORD e;
      res = reg_base_get_error(ph, &e);
      data->error = (UBYTE)e;
    }
  }

  return res;
}
