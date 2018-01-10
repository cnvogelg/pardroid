#include "types.h"
#include "arch.h"
#include "system.h"

#define DEBUG CONFIG_DEBUG_HANDLER

#include "debug.h"
#include "handler.h"
#include "status.h"
#include "channel.h"

static handler_ptr_t get_handler(u08 hid)
{
  return (handler_ptr_t)read_rom_rom_ptr(&handler_table[hid]);
}

void handler_init(u08 num)
{
  for(u08 hid=0;hid<num;hid++) {
    handler_ptr_t hnd = get_handler(hid);
    u08 flags = HANDLER_FLAG_NONE;
    u08 status = HANDLER_OK;

    // init func
    hnd_init_func_t f = (hnd_init_func_t)read_rom_rom_ptr(&hnd->init_func);
    if(f != 0) {
      status = f(hid);
      if(status == HANDLER_OK) {
        flags = HANDLER_FLAG_INIT;
      }
    } else {
      // auto init
      flags = HANDLER_FLAG_INIT;
    }

    // setup data
    handler_data_t *data = HANDLER_GET_DATA(hid);
    data->flags = flags;
    data->status = status;
    data->channel = CHANNEL_INVALID;
    data->mtu = read_rom_word(&hnd->mtu_max);
  }
}

void handler_work(u08 num)
{
  for(u08 hid=0;hid<num;hid++) {
    handler_ptr_t hnd = get_handler(hid);
    hnd_work_func_t f = (hnd_work_func_t)read_rom_rom_ptr(&hnd->work_func);
    if(f != 0) {
      handler_data_t *data = HANDLER_GET_DATA(hid);
      f(hid, data->flags);
    }
  }
}

u08 handler_open(u08 hid)
{
  /* valid index? */
  u08 max = HANDLER_GET_TABLE_SIZE();
  if(hid >= max) {
    return HANDLER_ERROR_INDEX;
  }

  /* not already open? */
  handler_data_t *data = HANDLER_GET_DATA(hid);
  if(data->flags & HANDLER_FLAG_OPEN) {
    return HANDLER_ALREADY_OPEN;
  }

  /* allocate channel for handler */
  u08 chn = channel_alloc(hid);
  if(chn == CHANNEL_INVALID) {
    return HANDLER_NO_CHANNEL;
  }
  data->channel = chn;

  /* call open func */
  handler_ptr_t hnd = get_handler(hid);
  hnd_open_func_t f = (hnd_open_func_t)read_rom_rom_ptr(&hnd->open_func);
  u08 status = HANDLER_OK;
  if(f != 0) {
    status = f(hid);
  }

  /* set flag */
  if(status == HANDLER_OK) {
    data->flags |= HANDLER_FLAG_OPEN;
  } else {
    /* free channel again */
    channel_free(chn);
  }
  data->status = status;
  return status;
}

u08 handler_close(u08 hid)
{
  /* valid index? */
  u08 max = HANDLER_GET_TABLE_SIZE();
  if(hid >= max) {
    return HANDLER_ERROR_INDEX;
  }

  /* not already open? */
  handler_data_t *data = HANDLER_GET_DATA(hid);
  if((data->flags & HANDLER_FLAG_OPEN) == HANDLER_FLAG_OPEN) {
    return HANDLER_CLOSED;
  }

  /* call close func */
  handler_ptr_t hnd = get_handler(hid);
  hnd_close_func_t f = (hnd_close_func_t)read_rom_rom_ptr(&hnd->close_func);
  if(f != 0) {
    f(hid);
  }

  /* free channel */
  channel_free(data->channel);

  /* remove flag */
  data->flags &= ~HANDLER_FLAG_OPEN;
  data->status = HANDLER_OK;
  data->channel = CHANNEL_INVALID;
  return HANDLER_OK;
}

u08 handler_read(u08 chn, u16 *size, u08 *buf)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    handler_ptr_t hnd = get_handler(chn);
    hnd_read_func_t f = (hnd_read_func_t)read_rom_rom_ptr(&hnd->read_func);
    return f(chn, size, buf);
  } else {
    return HANDLER_NO_FUNC;
  }
}

u08 handler_write(u08 chn, u16 size, u08 *buf)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    handler_ptr_t hnd = get_handler(chn);
    hnd_write_func_t f = (hnd_write_func_t)read_rom_rom_ptr(&hnd->write_func);
    return f(chn, size, buf);
  } else {
    return HANDLER_NO_FUNC;
  }
}

void handler_get_mtu(u08 chn, u16 *mtu_max, u16 *mtu_min)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    handler_ptr_t hnd = get_handler(chn);
    *mtu_max = read_rom_word(&hnd->mtu_max);
    *mtu_min = read_rom_word(&hnd->mtu_min);
  } else {
    *mtu_max = 0;
    *mtu_min = 0;
  }
}

void handler_set_status(u08 chn, u08 status)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    handler_data_t *data = HANDLER_GET_DATA(chn);
    data->status = status;
    u08 mask = 1 << chn;
    if(status == HANDLER_OK) {
      status_clear_error_mask(mask);
    } else {
      status_set_error_mask(mask);
    }
  }
}
