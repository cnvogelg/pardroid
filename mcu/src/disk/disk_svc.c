#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_DISK

#include "debug.h"

#include "pamela.h"
#include "pamela_req.h"

#include "disk.h"
#include "disk_map.h"
#include "disk_cmd.h"

#include "disk/wire.h"

#include "parbox/ports.h"

#include <stdlib.h>

/* how many concurrent user's of the cmd service are allowed */
#define NUM_CMD_SLOTS         1

/* you could use even more data slots if you want to access
   one disk slot via multiple clients */
#define NUM_DATA_SLOTS        DISK_MAP_SLOTS

#define DISK_MTU              512

/* state kept per slot in data service */
struct disk_data_slot {
  u08  disk_map_slot;
  u32  lba;
  u08  *buffer;
};
typedef struct disk_data_slot disk_data_slot_t;

static disk_data_slot_t slot_data[NUM_DATA_SLOTS];

// ----- disk control service -----

static u08 disk_ctl_handle(u08 chan, pamela_buf_t *buf)
{
  u08 slot_id = pamela_get_slot(chan);

  DS("disk_ctl_handle:#"); DB(chan); DC('/'); DB(slot_id); DNL;
  u16 ret_size = 0;
  u08 result = disk_cmd_handle(buf->data, buf->size, &ret_size);
  buf->size = ret_size;
  DS("disk_ctl_handle:ret:"); DB(result);
  return result;
}

// ----- disk data service -----

static u08 disk_data_open(u08 chn, u16 port)
{
  u08 pam_slot = pamela_get_slot(chn);
  disk_data_slot_t *slot = &slot_data[pam_slot];

  // derive disk map slot from the port number
  port -= DISK_DEFAULT_PORT + 1;
  u08 dm_slot = (u08)port;
  slot->disk_map_slot = dm_slot;

  DS("disk_data_open:#"); DB(chn); DC('/'); DB(pam_slot); DC('*'); DB(dm_slot); DNL;

  // allocate buffer
  slot->buffer = malloc(DISK_MTU);
  if(slot->buffer == NULL) {
    DS("no mem!"); DNL;
    return PAMELA_WIRE_ERROR_NO_MEM;
  }

  // try to open disk map slot
  u08 result = disk_map_open_slot(dm_slot);
  DS("disk_data_open:slot:ret:"); DB(result); DNL;
  if(result != DISK_OK) {
    free(slot->buffer);
    slot->buffer = NULL;
    return PAMELA_WIRE_ERROR_OBJ_NOT_FOUND;
  }

  return PAMELA_OK;
}

static u08 disk_data_close(u08 chn)
{
  u08 pam_slot = pamela_get_slot(chn);
  disk_data_slot_t *slot = &slot_data[pam_slot];
  u08 dm_slot = slot->disk_map_slot;

  DS("disk_data_close:#"); DB(chn); DC('/'); DB(pam_slot); DC('*'); DB(dm_slot); DNL;
  disk_map_close_slot(dm_slot);

  // free buffer
  free(slot->buffer);
  slot->buffer = NULL;

  slot->disk_map_slot = DISK_MAP_INVALID_SLOT;

  return PAMELA_OK;
}

static u08 disk_data_reset(u08 chn)
{
  // no reset for now
  return PAMELA_WIRE_ERROR_NOT_SUPPORTED;
}

static u08 disk_data_read_request(u08 chn, pamela_buf_t *buf)
{
  if(buf->size != DISK_MTU) {
    return PAMELA_WIRE_ERROR_WRONG_SIZE;
  }

  u08 pam_slot = pamela_get_slot(chn);
  disk_data_slot_t *slot = &slot_data[pam_slot];
  u08 dm_slot = slot->disk_map_slot;

  DS("disk_data_read:#"); DB(chn); DC('/'); DB(pam_slot); DC('*'); DB(dm_slot); DNL;

  buf->data = slot->buffer;

  // read buffer
  disk_handle_t *handle = disk_map_get_handle(dm_slot);
  u08 result = disk_read(handle, slot->lba, 1, slot->buffer);
  if(result != DISK_OK) {
    DS("disk_data_read:error:"); DB(result); DNL;
    return PAMELA_WIRE_ERROR_READ;
  }

  return PAMELA_OK;
}

static u08 disk_data_read_done(u08 chn, pamela_buf_t *buf)
{
  // n/a
  return PAMELA_OK;
}

static u08 disk_data_write_request(u08 chn, pamela_buf_t *buf)
{
  if(buf->size != DISK_MTU) {
    return PAMELA_WIRE_ERROR_WRONG_SIZE;
  }

  u08 pam_slot = pamela_get_slot(chn);
  disk_data_slot_t *slot = &slot_data[pam_slot];
  buf->data = slot->buffer;

  return PAMELA_OK;
}

static u08 disk_data_write_done(u08 chn, pamela_buf_t *buf)
{
  u08 pam_slot = pamela_get_slot(chn);
  disk_data_slot_t *slot = &slot_data[pam_slot];
  u08 dm_slot = slot->disk_map_slot;

  DS("disk_data_write:#"); DB(chn); DC('/'); DB(pam_slot); DC('*'); DB(dm_slot); DNL;

  disk_handle_t *handle = disk_map_get_handle(dm_slot);
  u08 result = disk_write(handle, slot->lba, 1, slot->buffer);
  if(result != DISK_OK) {
    DS("disk_data_write:error:"); DB(result); DNL;
    return PAMELA_WIRE_ERROR_READ;
  }

  return PAMELA_OK;
}

static void disk_data_seek(u08 chn, u32 pos)
{
  u08 pam_slot = pamela_get_slot(chn);
  disk_data_slot_t *slot = &slot_data[pam_slot];
  slot->lba = pos;
}

static u32 disk_data_tell(u08 chn)
{
  u08 pam_slot = pamela_get_slot(chn);
  disk_data_slot_t *slot = &slot_data[pam_slot];
  return slot->lba;
}

// ----- define disk_ctl handler -----
REQ_HANDLER_BEGIN(disk_svc_ctl, DISK_DEFAULT_PORT, NUM_CMD_SLOTS, DISK_WIRE_MAX_PACKET_SIZE)
  .open = pamela_req_open_malloc,
  .close = pamela_req_close_free,
  .handle = disk_ctl_handle,
REQ_HANDLER_END

// ----- define disk data handler
HANDLER_BEGIN(disk_svc_data)
  // parameters
  .config.port_begin = DISK_DEFAULT_PORT + 1,
  .config.port_end = DISK_DEFAULT_PORT + 1 + DISK_MAP_SLOTS,
  .config.def_mtu = DISK_MTU,
  .config.max_mtu = DISK_MTU,
  .config.max_slots = NUM_DATA_SLOTS,

  .open = disk_data_open,
  .close = disk_data_close,
  .reset = disk_data_reset,

  .seek = disk_data_seek,
  .tell = disk_data_tell,

  .read_request = disk_data_read_request,
  .read_done = disk_data_read_done,

  .write_request = disk_data_write_request,
  .write_done = disk_data_write_done,
HANDLER_END
