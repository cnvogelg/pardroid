#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "uart.h"
#include "uartutil.h"
#include "rominfo.h"
#include "system.h"
#include "led.h"
#include "timer.h"

#include "knok.h"
#include "proto.h"
#include "proto_shared.h"
#include "reg.h"

// action handler

void proto_api_action(u08 num)
{
  uart_send_pstring(PSTR("action:"));
  uart_send_hex_byte(num);
  uart_send_crlf();

  // triger signal
  if(num == 15) {
    uart_send_pstring(PSTR("signal!"));
    uart_send_crlf();
    proto_trigger_signal();
  }
  else if(num == 14) {
    uart_send_pstring(PSTR("busy:begin!"));
    uart_send_crlf();
    proto_busy_begin();
  }
  else if(num == 13) {
    uart_send_pstring(PSTR("busy:end!"));
    uart_send_crlf();
    proto_busy_end();
  }
}

// function handler

u16  proto_api_wfunc_read(u08 num)
{
  u16 val = 0xbeef;
  if(num < PROTO_WFUNC_USER) {
    val = reg_wfunc_read_handle(num);
  }
  uart_send_pstring(PSTR("wfunc_read:"));
  uart_send_hex_byte(num);
  uart_send('=');
  uart_send_hex_word(val);
  uart_send_crlf();
  return val;
}

void proto_api_wfunc_write(u08 num, u16 val)
{
  if(num < PROTO_WFUNC_USER) {
    reg_wfunc_write_handle(num, val);
  }
  uart_send_pstring(PSTR("wfunc_write:"));
  uart_send_hex_byte(num);
  uart_send('=');
  uart_send_hex_word(val);
  uart_send_crlf();
}

u32 proto_api_lfunc_read(u08 num)
{
  uart_send_pstring(PSTR("lfunc_read:"));
  uart_send_hex_byte(num);
  uart_send_crlf();
  return 0xcafebabe;
}

void proto_api_lfunc_write(u08 num, u32 val)
{
  uart_send_pstring(PSTR("lfunc_write:"));
  uart_send_hex_byte(num);
  uart_send('=');
  uart_send_hex_long(val);
  uart_send_crlf();
}

// registers

void reg_api_set_value(u08 range, u08 reg, u16 value)
{
  uart_send_pstring(PSTR("reg_set:"));
  uart_send_hex_byte(range);
  uart_send(':');
  uart_send_hex_byte(reg);
  uart_send('=');
  uart_send_hex_word(value);
  uart_send_crlf();
}

u16 reg_api_get_value(u08 range, u08 reg)
{
  u16 val = 0xdead;
  uart_send_pstring(PSTR("reg_get:"));
  uart_send_hex_byte(range);
  uart_send(':');
  uart_send_hex_byte(reg);
  uart_send('=');
  uart_send_hex_word(val);
  uart_send_crlf();
  return val;
}

// messages

u08 buf[512];

u08 *proto_api_read_msg_prepare(u08 chan,u16 *size, u16 *crc)
{
  *size = 256;
  *crc = 0xdead;
  uart_send_pstring(PSTR("msg_read:{"));
  uart_send_hex_byte(chan);
  return buf;
}

void proto_api_read_msg_done(u08 chan)
{
  uart_send('}');
  uart_send_crlf();
}

u08 *proto_api_write_msg_prepare(u08 chan,u16 *max_size)
{
  *max_size = 256;
  uart_send_pstring(PSTR("msg_write:{"));
  uart_send_hex_byte(chan);
  return buf;
}

void proto_api_write_msg_done(u08 chan,u16 size, u16 crc)
{
  uart_send_hex_word(size);
  uart_send(',');
  uart_send_hex_word(crc);
  uart_send('}');
  uart_send_crlf();
}

int main(void)
{
  system_init();
  //led_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-proto!"));
  uart_send_crlf();

  rom_info();

  knok_main();

  proto_init();
  proto_first();
  while(1) {
      proto_handle();
      system_wdt_reset();
  }

  return 0;
}
