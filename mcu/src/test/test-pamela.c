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
#include "system.h"
#include "timer.h"
#include "func.h"
#include "status.h"

#include "pamela.h"
#include "test-pamela.h"

static u16 wfunc_val = 0;
static u32 lfunc_val = 0;

#define MAX_WORDS 512
static u08 buf[MAX_WORDS * 2];
static u16 buf_words = MAX_WORDS;

static u08 enter_busy_loop = 0;

void busy_loop(void)
{
  uart_send_pstring(PSTR("busy:begin"));
  status_set_busy();

  // wait for a second
  for(int i=0;i<5;i++) {
    system_wdt_reset();
    uart_send('.');
    timer_delay(200);
  }

  status_clr_busy();
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
  if(num == TEST_PAMELA_ACTION_TRIGGER_SIGNAL) {
    uart_send_pstring(PSTR("signal!"));
    uart_send_crlf();
    proto_trigger_signal();
  }
  else if(num == TEST_PAMELA_ACTION_BUSY_LOOP) {
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
    val = func_read_word(num);
  } else {
    switch(num) {
      case TEST_PAMELA_WFUNC_MAX_BYTES:
        val = MAX_WORDS * 2;
        break;
      case TEST_PAMELA_WFUNC_BUF_WORDS:
        val = buf_words;
        break;
      case TEST_PAMELA_WFUNC_TEST_VALUE:
        val = wfunc_val;
        break;
      default:
        val = 0;
        break;
    }
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
    func_write_word(num, val);
  } else {
    switch(num) {
      case TEST_PAMELA_WFUNC_BUF_WORDS:
        buf_words = val;
        break;
      case TEST_PAMELA_WFUNC_TEST_VALUE:
        wfunc_val = val;
        break;
    }
  }
  uart_send_pstring(PSTR("wfunc_write:"));
  uart_send_hex_byte(num);
  uart_send('=');
  uart_send_hex_word(val);
  uart_send_crlf();
}

u32 proto_api_lfunc_read(u08 num)
{
  u32 val;
  if(num < PROTO_LFUNC_USER) {
    val = func_read_long(num);
  } else {
    switch(num) {
      case TEST_PAMELA_LFUNC_TEST_VALUE:
        val = lfunc_val;
        break;
      default:
        val = 0;
        break;
    }
  }
  uart_send_pstring(PSTR("lfunc_read:"));
  uart_send_hex_byte(num);
  uart_send('=');
  uart_send_hex_long(val);
  uart_send_crlf();
  return val;
}

void proto_api_lfunc_write(u08 num, u32 val)
{
  if(num < PROTO_LFUNC_USER) {
    func_write_long(num, val);
  } else {
    switch(num) {
      case TEST_PAMELA_LFUNC_SET_STATUS:
        status_set_mask(val);
        break;
      case TEST_PAMELA_LFUNC_TEST_VALUE:
        lfunc_val = val;
        break;
    }
  }
  uart_send_pstring(PSTR("lfunc_write:"));
  uart_send_hex_byte(num);
  uart_send('=');
  uart_send_hex_long(val);
  uart_send_crlf();
}

// messages

u16 proto_api_read_msg_size(u08 chan)
{
  return buf_words;
}

u08 *proto_api_read_msg_begin(u08 chan,u16 num_words)
{
  uart_send_pstring(PSTR("msg_read:{"));
  uart_send_hex_byte(chan);
  uart_send(':');
  uart_send_hex_word(buf_words);
  return buf;
}

void proto_api_read_msg_done(u08 chan,u16 num_words)
{
  uart_send('}');
  uart_send_crlf();
}

void proto_api_write_msg_size(u08 chan, u16 size)
{
  buf_words = size;
}

u08 *proto_api_write_msg_begin(u08 chan,u16 num_words)
{
  uart_send_pstring(PSTR("msg_write:{"));
  uart_send_hex_byte(chan);
  uart_send(':');
  uart_send_hex_word(buf_words);
  return buf;
}

void proto_api_write_msg_done(u08 chan,u16 num_words)
{
  uart_send('}');
  uart_send_crlf();
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
