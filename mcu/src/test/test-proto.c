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
#include "proto_api.h"

#include "fwid.h"
#include "fw_info.h"

FW_INFO(FWID_TEST_PROTO, VERSION_TAG)

#define MAX_WORDS  512
u08 buf[MAX_WORDS * 2];

u16 test_flags = 0;
u16 rx_size = MAX_WORDS;
u16 tx_size = MAX_WORDS;
u32 rx_offset = 0;
u32 tx_offset = 0;
u16 word_val = 0x4711;
u32 long_val = 0xdeadbeef;
u08 enter_busy = 0;

static void busy_loop(void)
{
  uart_send_pstring(PSTR("busy:begin"));
  proto_set_busy();

  // wait for a second
  for(int i=0;i<5;i++) {
    system_wdt_reset();
    uart_send('.');
    timer_delay(200);
  }

  proto_clr_busy();
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
  switch(num) {
    case PROTO_ACTION_TEST_SIGNAL:
      uart_send_pstring(PSTR("signal!"));
      uart_send_crlf();
      proto_trigger_signal();
      break;
    case PROTO_ACTION_TEST_BUSY_LOOP:
      enter_busy = 1;
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
    case PROTO_WFUNC_READ_TEST_FLAGS:
      val = test_flags;
      break;
    case PROTO_WFUNC_READ_TEST_VALUE:
      val = word_val;
      break;
    case PROTO_WFUNC_READ_TEST_GET_TX_SIZE:
      val = tx_size;
      break;
    case PROTO_WFUNC_READ_TEST_MAX_WORDS:
      val = MAX_WORDS;
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
    case PROTO_WFUNC_WRITE_TEST_FLAGS:
      test_flags = val;
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
    case PROTO_LFUNC_READ_TEST_RX_OFFSET:
      val = rx_offset;
      break;
    case PROTO_LFUNC_READ_TEST_TX_OFFSET:
      val = tx_offset;
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

u08 *proto_api_read_msg_begin(u08 chan, u16 *size)
{
  *size = rx_size;
  uart_send_pstring(PSTR("msg_read:{"));
  uart_send_hex_byte(chan);
  uart_send('+');
  uart_send_hex_word(*size);
  return buf;
}

void proto_api_read_msg_done(u08 chan)
{
  uart_send('}');
  uart_send_crlf();
}

u08 *proto_api_write_msg_begin(u08 chan,u16 *size)
{
  *size = tx_size;
  uart_send_pstring(PSTR("msg_write:{"));
  uart_send_hex_byte(chan);
  uart_send('+');
  uart_send_hex_word(*size);
  return buf;
}

void proto_api_write_msg_done(u08 chan)
{
  uart_send('}');
  uart_send_crlf();
}

// extended commands

void proto_api_chn_set_rx_offset(u08 chan, u32 offset)
{
  uart_send_pstring(PSTR("rx_offset="));
  uart_send_hex_long(rx_offset);
  uart_send_crlf();
  rx_offset = offset;
}

void proto_api_chn_set_tx_offset(u08 chan, u32 offset)
{
  uart_send_pstring(PSTR("tx_offset="));
  uart_send_hex_long(tx_offset);
  uart_send_crlf();
  tx_offset = offset;
}

u16  proto_api_chn_get_rx_size(u08 chan)
{
  uart_send_pstring(PSTR("rx_size:"));
  uart_send_hex_word(rx_size);
  uart_send_crlf();
  return rx_size;
}

void proto_api_chn_set_rx_size(u08 chan, u16 size)
{
  uart_send_pstring(PSTR("rx_size="));
  uart_send_hex_word(size);
  uart_send_crlf();
  if(rx_size <= MAX_WORDS) {
    rx_size = size;
  }
}

void proto_api_chn_set_tx_size(u08 chan, u16 size)
{
  uart_send_pstring(PSTR("tx_size="));
  uart_send_hex_word(size);
  uart_send_crlf();
  if(tx_size <= MAX_WORDS) {
    tx_size = size;
  }
}

void proto_api_chn_request_rx(u08 chan)
{
  uart_send_pstring(PSTR("request_rx"));
  uart_send_crlf();
  test_flags |= PROTO_TEST_FLAGS_RX_REQUEST;
}

void proto_api_chn_cancel_rx(u08 chan)
{
  uart_send_pstring(PSTR("cancel_rx"));
  uart_send_crlf();
  test_flags |= PROTO_TEST_FLAGS_RX_CANCEL;
}

void proto_api_chn_cancel_tx(u08 chan)
{
  uart_send_pstring(PSTR("cancel_tx"));
  uart_send_crlf();
  test_flags |= PROTO_TEST_FLAGS_TX_CANCEL;
}

// ----- main -----

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
      proto_handle();
      system_wdt_reset();

      if(enter_busy) {
        busy_loop();
        enter_busy = 0;
      }
  }

  return 0;
}
