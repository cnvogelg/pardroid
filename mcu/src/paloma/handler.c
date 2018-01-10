#include "types.h"
#include "arch.h"
#include "system.h"

#define DEBUG CONFIG_DEBUG_HANDLER

#include "debug.h"
#include "handler.h"
#include "status.h"

static handler_ptr_t get_handler(u08 num)
{
  return (handler_ptr_t)read_rom_rom_ptr(&handler_table[num]);
}

void handler_init(u08 num)
{
  for(u08 chn=0;chn<num;chn++) {
    handler_ptr_t hnd = get_handler(chn);
    hnd_init_func_t f = (hnd_init_func_t)read_rom_rom_ptr(&hnd->init_func);
    u08 flags = HANDLER_FLAG_NONE;
    u08 status = HANDLER_OK;
    if(f != 0) {
      status = f(chn);
      if(status == HANDLER_OK) {
        flags = HANDLER_FLAG_INIT;
      }
    } else {
      /* auto init */
      flags = HANDLER_FLAG_INIT;
    }
    handler_data_t *data = HANDLER_GET_DATA(chn);
    data->flags = flags;
    data->status = status;
    /* set default values */
    data->mtu = read_rom_word(&hnd->mtu_max);
  }
}

void handler_work(u08 num)
{
  for(u08 chn=0;chn<num;chn++) {
    handler_ptr_t hnd = get_handler(chn);
    hnd_work_func_t f = (hnd_work_func_t)read_rom_rom_ptr(&hnd->work_func);
    if(f != 0) {
      handler_data_t *data = HANDLER_GET_DATA(chn);
      f(chn, data->flags);
    }
  }
}

u08 handler_open(u08 chn)
{
  /* valid index? */
  u08 max = HANDLER_GET_TABLE_SIZE();
  if(chn >= max) {
    return HANDLER_ERROR_INDEX;
  }

  /* not already open? */
  handler_data_t *data = HANDLER_GET_DATA(chn);
  if(data->flags & HANDLER_FLAG_OPEN) {
    return HANDLER_ALREADY_OPEN;
  }

  /* call open func */
  handler_ptr_t hnd = get_handler(chn);
  hnd_open_func_t f = (hnd_open_func_t)read_rom_rom_ptr(&hnd->open_func);
  u08 status = HANDLER_OK;
  if(f != 0) {
    status = f(chn);
  }

  /* set flag */
  if(status == HANDLER_OK) {
    data->flags |= HANDLER_FLAG_OPEN;
  }
  data->status = status;
  return status;
}

u08 handler_close(u08 chn)
{
  /* valid index? */
  u08 max = HANDLER_GET_TABLE_SIZE();
  if(chn >= max) {
    return HANDLER_ERROR_INDEX;
  }

  /* not already open? */
  handler_data_t *data = HANDLER_GET_DATA(chn);
  if((data->flags & HANDLER_FLAG_OPEN) == HANDLER_FLAG_OPEN) {
    return HANDLER_CLOSED;
  }

  /* call close func */
  handler_ptr_t hnd = get_handler(chn);
  hnd_close_func_t f = (hnd_close_func_t)read_rom_rom_ptr(&hnd->close_func);
  if(f != 0) {
    f(chn);
  }

  /* remove flag */
  data->flags &= ~HANDLER_FLAG_OPEN;
  data->status = HANDLER_OK;
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