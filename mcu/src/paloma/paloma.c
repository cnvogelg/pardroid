#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_PALOMA

#include "debug.h"

#include "pamela.h"
#include "paloma.h"
#include "parbox/ports.h"
#include "paloma/wire.h"
#include "paloma_cmd.h"

#include <stdlib.h>

#define NUM_SLOTS 2

// ----- paloma service -----

static u08 paloma_handle(u08 chan, u08 state, pamela_buf_t *buf)
{
  u08 slot_id = pamela_get_slot(chan);

  DS("paloma_handle:"); DB(chan); DC('/'); DB(slot_id); DNL;
  u08 ret_size = 0;
  paloma_cmd_handle(buf->data, buf->size, &ret_size);
  buf->size = ret_size;

  return PAMELA_HANDLER_OK;
}

// ----- define paloma handler -----
REQ_HANDLER_BEGIN(paloma, PALOMA_DEFAULT_PORT, NUM_SLOTS, PALOMA_WIRE_MAX_PACKET_SIZE)
  .open = pamela_req_open_malloc,
  .close = pamela_req_close_free,
  .handle = paloma_handle,
REQ_HANDLER_END
