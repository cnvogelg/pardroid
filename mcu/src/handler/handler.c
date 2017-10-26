#include "types.h"
#include "arch.h"
#include "debug.h"
#include "system.h"
#include "handler.h"

static handler_ptr_t get_handler(u08 num)
{
  rom_pchar ptr = read_rom_rom_ptr(&handler_table[num]);
  return (handler_ptr_t)ptr;
}

u08 handler_init(u08 chn)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    handler_ptr_t hnd = get_handler(chn);
    rom_pchar ptr = read_rom_rom_ptr(&hnd->init_func);
    init_func_t f = (init_func_t)ptr;
    if(f != 0) {
      return f(chn);
    } else {
      return HANDLER_OK;
    }
  } else {
    return HANDLER_ERROR_INDEX;
  }
}

void handler_work(u08 chn, u08 flags)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    handler_ptr_t hnd = get_handler(chn);
    rom_pchar ptr = read_rom_rom_ptr(&hnd->work_func);
    work_func_t f = (work_func_t)ptr;
    if(f != 0) {
      f(chn, flags);
    }
  }
}

u08 handler_open(u08 chn)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    handler_ptr_t hnd = get_handler(chn);
    rom_pchar ptr = read_rom_rom_ptr(&hnd->open_func);
    open_func_t f = (open_func_t)ptr;
    if(f != 0) {
      return f(chn);
    } else {
      return HANDLER_OK;
    }
  } else {
    return HANDLER_ERROR_INDEX;
  }
}

void handler_close(u08 chn)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    handler_ptr_t hnd = get_handler(chn);
    rom_pchar ptr = read_rom_rom_ptr(&hnd->close_func);
    close_func_t f = (close_func_t)ptr;
    if(f != 0) {
      return f(chn);
    }
  }
}

u08 *handler_read_msg_prepare(u08 chn, u16 *size)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    handler_ptr_t hnd = get_handler(chn);
    rom_pchar ptr = read_rom_rom_ptr(&hnd->read_msg_prepare);
    read_msg_prepare_func_t f = (read_msg_prepare_func_t)ptr;
    return f(chn, size);
  } else {
    *size = 0;
    return 0;
  }
}

void handler_read_msg_done(u08 chn, u08 status)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    handler_ptr_t hnd = get_handler(chn);
    rom_pchar ptr = read_rom_rom_ptr(&hnd->read_msg_done);
    read_msg_done_func_t f = (read_msg_done_func_t)ptr;
    return f(chn, status);
  }
}

u08 *handler_write_msg_prepare(u08 chn, u16 *max_size)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    handler_ptr_t hnd = get_handler(chn);
    rom_pchar ptr = read_rom_rom_ptr(&hnd->write_msg_prepare);
    write_msg_prepare_func_t f = (write_msg_prepare_func_t)ptr;
    return f(chn, max_size);
  } else {
    *max_size = 0;
    return 0;
  }
}

void handler_write_msg_done(u08 chn, u16 size)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    handler_ptr_t hnd = get_handler(chn);
    rom_pchar ptr = read_rom_rom_ptr(&hnd->write_msg_done);
    write_msg_done_func_t f = (write_msg_done_func_t)ptr;
    return f(chn, size);
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

/* aliases for proto API functions */
u08 *proto_api_read_msg_prepare(u08 chn, u16 *size) __attribute__ ((weak, alias("handler_read_msg_prepare")));
void proto_api_read_msg_done(u08 chn) __attribute__ ((weak, alias("handler_read_msg_done")));
u08 *proto_api_write_msg_prepare(u08 chn, u16 *max_size) __attribute__ ((weak, alias("handler_write_msg_prepare")));
void proto_api_write_msg_done(u08 chn) __attribute__ ((weak, alias("handler_write_msg_done")));
