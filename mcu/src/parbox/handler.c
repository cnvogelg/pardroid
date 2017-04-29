#include "types.h"
#include "arch.h"
#include "debug.h"
#include "system.h"
#include "handler.h"

void handler_init(void)
{
  u08 max = read_rom_char(&handler_table_size);
  for(u08 i=0;i<max;i++) {
    rom_pchar ptr = read_rom_rom_ptr(&handler_table[i].init_func);
    init_func_t f = (init_func_t)ptr;
    f();
  }
}

void handler_work(void)
{
  u08 max = read_rom_char(&handler_table_size);
  for(u08 i=0;i<max;i++) {
    rom_pchar ptr = read_rom_rom_ptr(&handler_table[i].work_func);
    work_func_t f = (work_func_t)ptr;
    f();
  }
}

u08 *handler_read_msg_prepare(u08 chn, u16 *size)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    rom_pchar ptr = read_rom_rom_ptr(&handler_table[chn].read_msg_prepare);
    read_msg_prepare_func_t f = (read_msg_prepare_func_t)ptr;
    return f(size);
  } else {
    *size = 0;
    return 0;
  }
}

void handler_read_msg_done(u08 chn)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    rom_pchar ptr = read_rom_rom_ptr(&handler_table[chn].read_msg_done);
    read_msg_done_func_t f = (read_msg_done_func_t)ptr;
    return f();
  }
}

u08 *handler_write_msg_prepare(u08 chn, u16 *max_size)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    rom_pchar ptr = read_rom_rom_ptr(&handler_table[chn].write_msg_prepare);
    write_msg_prepare_func_t f = (write_msg_prepare_func_t)ptr;
    return f(max_size);
  } else {
    *max_size = 0;
    return 0;
  }
}

void handler_write_msg_done(u08 chn, u16 size)
{
  u08 max = read_rom_char(&handler_table_size);
  if(chn < max) {
    rom_pchar ptr = read_rom_rom_ptr(&handler_table[chn].write_msg_done);
    write_msg_done_func_t f = (write_msg_done_func_t)ptr;
    return f(size);
  }
}

/* aliases for proto API functions */
u08 *proto_api_read_msg_prepare(u08 chn, u16 *size) __attribute__ ((weak, alias("handler_read_msg_prepare")));
void proto_api_read_msg_done(u08 chn) __attribute__ ((weak, alias("handler_read_msg_done")));
u08 *proto_api_write_msg_prepare(u08 chn, u16 *max_size) __attribute__ ((weak, alias("handler_write_msg_prepare")));
void proto_api_write_msg_done(u08 chn) __attribute__ ((weak, alias("handler_write_msg_done")));
