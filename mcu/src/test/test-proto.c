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
#include "fwid.h"
#include "fw_info.h"

FW_INFO(FWID_TEST_PROTO, VERSION_TAG)

#define MAX_WORDS  512
u08 buf[MAX_WORDS * 2];
u08 use_spi = 0;
u16 num_words = MAX_WORDS;
u16 word_val = 0x4711;
u32 long_val = 0xdeadbeef;

// action handler

void proto_api_action(u08 num)
{
  uart_send_pstring(PSTR("action:"));
  uart_send_hex_byte(num);
  uart_send_crlf();

  // triger signal
  switch(num) {
    case PROTO_ACTION_TEST_SIGNAL:
      uart_send_pstring(PSTR("signal!"));
      uart_send_crlf();
      proto_trigger_signal();
      break;
    case PROTO_ACTION_TEST_BUSY_BEGIN:
      uart_send_pstring(PSTR("busy:begin!"));
      uart_send_crlf();
      proto_set_busy();
      break;
    case PROTO_ACTION_TEST_BUSY_END:
      uart_send_pstring(PSTR("busy:end!"));
      uart_send_crlf();
      proto_clr_busy();
      break;
  }
}

// function handler

u16  proto_api_wfunc_read(u08 num)
{
  u16 val = 0xbeef;
  switch(num) {
    case PROTO_WFUNC_READ_TEST_FW_ID:
      val = FW_GET_ID();
      break;
    case PROTO_WFUNC_READ_TEST_FW_VERSION:
      val = FW_GET_VERSION();
      break;
    case PROTO_WFUNC_READ_TEST_MACHTAG:
      val = FW_GET_MACHTAG();
      break;
    case PROTO_WFUNC_READ_TEST_NUM_WORDS:
      val = num_words;
      break;
    case PROTO_WFUNC_READ_TEST_USE_SPI:
      val = use_spi;
      break;
    case PROTO_WFUNC_READ_TEST_VALUE:
      val = word_val;
      break;
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
  switch(num) {
    case PROTO_WFUNC_WRITE_TEST_NUM_WORDS:
      if(val <= MAX_WORDS) {
        num_words = val;
      }
      break;
    case PROTO_WFUNC_WRITE_TEST_USE_SPI:
      use_spi = (val > 0) ? 1 : 0;
      break;
    case PROTO_WFUNC_WRITE_TEST_VALUE:
      word_val = val;
      break;
  }
  uart_send_pstring(PSTR("wfunc_write:"));
  uart_send_hex_byte(num);
  uart_send('=');
  uart_send_hex_word(val);
  uart_send_crlf();
}

u32 proto_api_lfunc_read(u08 num)
{
  u32 val = 0xcafebabe;
  switch(num) {
    case PROTO_LFUNC_READ_TEST_VALUE:
      val = long_val;
      break;
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
  switch(num) {
    case PROTO_LFUNC_WRITE_TEST_VALUE:
      long_val = val;
      break;
  }
  uart_send_pstring(PSTR("lfunc_write:"));
  uart_send_hex_byte(num);
  uart_send('=');
  uart_send_hex_long(val);
  uart_send_crlf();
}

// message i/o

u16 proto_api_read_msg_size(u08 chan)
{
  return num_words;
}

u08 *proto_api_read_msg_begin(u08 chan,u16 size)
{
  uart_send_pstring(PSTR("msg_read:{"));
  uart_send_hex_byte(chan);
  return buf;
}

void proto_api_read_msg_done(u08 chan, u16 size)
{
  uart_send('}');
  uart_send_crlf();
}

u16 proto_api_write_msg_size(u08 chan, u16 size)
{
  return num_words;
}

u08 *proto_api_write_msg_begin(u08 chan,u16 size)
{
  uart_send_pstring(PSTR("msg_write:{"));
  uart_send_hex_byte(chan);
  return buf;
}

void proto_api_write_msg_done(u08 chan, u16 size)
{
  uart_send('}');
  uart_send_crlf();
}

u32 proto_api_read_offset(u08 chan)
{
  return 0;
}

void proto_api_write_offset(u08 chan, u32 off)
{
}

u16 proto_api_read_mtu(u08 chan)
{
  return 256;
}

void proto_api_write_mtu(u08 chan, u16 mtu)
{
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
  proto_first_cmd();
  while(1) {
      proto_handle_mini();
      system_wdt_reset();
  }

  return 0;
}
