#include "types.h"
#include "arch.h"
#include "autoconf.h"

#include "pamela.h"
#include "paloma.h"
#include "parbox/ports.h"
#include "paloma/wire.h"
#include "paloma_cmd.h"

#define NUM_SLOTS 2

#define PALOMA_MODE_IDLE        0
#define PALOMA_MODE_REQUEST     1
#define PALOMA_MODE_GOT_CMD     2
#define PALOMA_MODE_REPLY       3

struct slot_data {
  u08 buffer[PALOMA_WIRE_MAX_PACKET_SIZE];
  u08 mode;
  u08 last_cmd;
  u08 reply_size;
};
typedef struct slot_data slot_data_t;
static struct slot_data slots[NUM_SLOTS];

// ----- paloma service -----

static u08 paloma_open(u08 slot, u08 chan, u16 port)
{
  slot_data_t *data = &slots[slot];
  data->mode = PALOMA_MODE_IDLE;

  return PAMELA_OK;
}

static u08 paloma_close(u08 slot)
{
  return PAMELA_OK;
}

static u08 paloma_reset(u08 slot)
{
  slot_data_t *data = &slots[slot];
  data->mode = PALOMA_MODE_IDLE;

  return PAMELA_OK;
}

// ----- read -----

static u08 paloma_read_request(u08 slot, u08 **buf, u16 *size)
{
  slot_data_t *data = &slots[slot];

  /* make sure we got a cmd */
  if(data->mode != PALOMA_MODE_GOT_CMD) {
    return PAMELA_ERROR;
  }

  data->mode = PALOMA_MODE_REPLY;

  *buf = &data->buffer[0];
  *size = data->reply_size;

  return PAMELA_OK;
}

static void paloma_read_done(u08 slot, u08 *buf, u16 size)
{
  slot_data_t *data = &slots[slot];
  data->mode = PALOMA_MODE_IDLE;
}

// ----- write -----

static u08 paloma_write_request(u08 slot, u08 **buf, u16 *size)
{
  slot_data_t *data = &slots[slot];

  /* make sure we are idle and accept a new request */
  if(data->mode != PALOMA_MODE_IDLE) {
    return PAMELA_ERROR;
  }

  data->mode = PALOMA_MODE_REQUEST;

  *buf = &data->buffer[0];

  return PAMELA_OK;
}

static void paloma_write_done(u08 slot, u08 *buf, u16 size)
{
  slot_data_t *data = &slots[slot];

  paloma_cmd_handle(buf, size, &data->reply_size);

  data->mode = PALOMA_MODE_GOT_CMD;
}

// ----- define paloma handler -----
HANDLER_BEGIN(paloma)
  // parameters
  .config.port_begin = PALOMA_DEFAULT_PORT,
  .config.port_end = PALOMA_DEFAULT_PORT,
  .config.def_mtu = PALOMA_WIRE_MAX_PACKET_SIZE,
  .config.max_mtu = PALOMA_WIRE_MAX_PACKET_SIZE,
  .config.max_slots = NUM_SLOTS,

  .open = paloma_open,
  .close = paloma_close,
  .reset = paloma_reset,

  .read_request = paloma_read_request,
  .read_done = paloma_read_done,
  .write_request = paloma_write_request,
  .write_done = paloma_write_done,
HANDLER_END

void paloma_init(void)
{
  pamela_add_handler(&paloma);
}
