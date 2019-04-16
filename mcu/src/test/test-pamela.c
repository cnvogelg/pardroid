#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "uart.h"
#include "uartutil.h"
#include "rominfo.h"
#include "fwid.h"
#include "proto.h"
#include "reg.h"
#include "system.h"
#include "timer.h"

#include "pamela.h"

static u16 wfunc_val = 0;
static u32 lfunc_val = 0;

#define MAX_WORDS 512
static u08 buf[MAX_WORDS * 2];
static u16 buf_words = MAX_WORDS;
static u16 buf_crc = 0;

static u08 enter_busy_loop = 0;

void busy_loop(void)
{
  uart_send_pstring(PSTR("busy:begin"));
  proto_busy_begin();

  // wait for a second
  for(int i=0;i<5;i++) {
    system_wdt_reset();
    uart_send('.');
    timer_delay(200);
  }

  proto_busy_end();
  uart_send_pstring(PSTR("end"));
  uart_send_crlf();
}

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
    uart_send_pstring(PSTR("busy!"));
    uart_send_crlf();
    enter_busy_loop = 1;
  }
}

// function handler

u16  proto_api_wfunc_read(u08 num)
{
  u16 val;
  if(num < PROTO_WFUNC_USER) {
    val = reg_wfunc_read_handle(num);
  } else if(num == (PROTO_WFUNC_USER+1)) {
    val = MAX_WORDS * 2; 
  } else {
    val = wfunc_val;
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
  } else if(num == (PROTO_WFUNC_USER+2)) {
    buf_words = val; 
  } else {
    wfunc_val = val;
  }
  uart_send_pstring(PSTR("wfunc_write:"));
  uart_send_hex_byte(num);
  uart_send('=');
  uart_send_hex_word(val);
  uart_send_crlf();
}

u32 proto_api_lfunc_read(u08 num)
{
  u32 val = lfunc_val;
  uart_send_pstring(PSTR("lfunc_read:"));
  uart_send_hex_byte(num);
  uart_send('=');
  uart_send_hex_long(val);
  uart_send_crlf();
  return val;
}

void proto_api_lfunc_write(u08 num, u32 val)
{
  lfunc_val = val;
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

u08 *proto_api_read_msg_prepare(u08 chan,u16 *size, u16 *crc)
{
  *size = buf_words;
  *crc = buf_crc;
  uart_send_pstring(PSTR("msg_read:{"));
  uart_send_hex_byte(chan);
  uart_send(':');
  uart_send_hex_word(buf_words);
  uart_send(',');
  uart_send_hex_word(buf_crc);
  return buf;
}

void proto_api_read_msg_done(u08 chan)
{
  uart_send('}');
  uart_send_crlf();
}

u08 *proto_api_write_msg_prepare(u08 chan,u16 *max_size)
{
  *max_size = MAX_WORDS;
  uart_send_pstring(PSTR("msg_write:{"));
  uart_send_hex_byte(chan);
  uart_send(':');
  return buf;
}

void proto_api_write_msg_done(u08 chan,u16 size, u16 crc)
{
  uart_send_hex_word(size);
  uart_send(',');
  uart_send_hex_word(crc);
  uart_send('}');
  uart_send_crlf();
  buf_words = size;
  buf_crc = crc;
}

int main(void)
{
  system_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-pamela!"));
  uart_send_crlf();

  rom_info();

  pamela_init();

  while(1) {
    system_wdt_reset();
    pamela_handle();

    // make a busy wait?
    if(enter_busy_loop) {
      busy_loop();
      enter_busy_loop = 0;
    }
  }

  return 0;
}
