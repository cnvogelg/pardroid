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
#include "spi.h"

#include "pamela.h"
#include "test-pamela.h"
#include "enc28j60.h"
#include "fw.h"
#include "fwid.h"

FW_INFO(FWID_TEST_PAMELA, VERSION_TAG)

static u16 wfunc_val = 0;
static u32 lfunc_val = 0;
static u32 offset = 0;

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
  if(num < PROTO_WFUNC_READ_USER) {
    val = func_read_word(num);
  } else {
    switch(num) {
      case TEST_PAMELA_WFUNC_READ_TEST_VALUE:
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
  if(num < PROTO_WFUNC_WRITE_USER) {
    func_write_word(num, val);
  } else {
    switch(num) {
      case TEST_PAMELA_WFUNC_WRITE_TEST_VALUE:
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
  if(num < PROTO_LFUNC_READ_USER) {
    val = func_read_long(num);
  } else {
    switch(num) {
      case TEST_PAMELA_LFUNC_READ_TEST_VALUE:
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
  if(num < PROTO_LFUNC_WRITE_USER) {
    func_write_long(num, val);
  } else {
    switch(num) {
      case TEST_PAMELA_LFUNC_SET_STATUS:
        status_set_mask(val);
        break;
      case TEST_PAMELA_LFUNC_WRITE_TEST_VALUE:
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
  if(chan == TEST_PAMELA_SPI_CHANNEL) {
    uart_send('S');
    spi_enable_cs0();
    return NULL;
  } 
  else if(chan == TEST_PAMELA_NET_CHANNEL) {
    uart_send('N');
    enc28j60_test_begin_rx();
    return NULL;
  }
  else {
    return buf;
  }
}

void proto_api_read_msg_done(u08 chan,u16 num_words)
{
  if(chan == TEST_PAMELA_SPI_CHANNEL) {
    uart_send('S');
    spi_disable_cs0();
  }
  else if(chan == TEST_PAMELA_NET_CHANNEL) {
    uart_send('N');
    enc28j60_test_end_rx();
  }
  uart_send('}');
  uart_send_crlf();
  /* clear rx pending */
  status_clr_rx_pending(chan);
}

u16 proto_api_write_msg_size(u08 chan, u16 size)
{
  if(size <= MAX_WORDS) {
    buf_words = size;
    return size;
  } else {
    return 0;
  }
}

u08 *proto_api_write_msg_begin(u08 chan,u16 num_words)
{
  uart_send_pstring(PSTR("msg_write:{"));
  uart_send_hex_byte(chan);
  uart_send(':');
  uart_send_hex_word(buf_words);
  if(chan == TEST_PAMELA_SPI_CHANNEL) {
    uart_send('S');
    spi_enable_cs0();
    return NULL;
  } 
  else if(chan == TEST_PAMELA_NET_CHANNEL) {
    uart_send('N');
    enc28j60_test_begin_tx();
    return NULL;
  }
  else {
    return buf;
  }
}

void proto_api_write_msg_done(u08 chan,u16 num_words)
{
  if(chan == TEST_PAMELA_SPI_CHANNEL) {
    uart_send('S');
    spi_disable_cs0();
  }
  else if(chan == TEST_PAMELA_NET_CHANNEL) {
    uart_send('N');
    enc28j60_test_end_tx();
  }
  uart_send('}');

  /* set rx pending for echo */
  status_set_rx_pending(chan);
}

u32 proto_api_read_offset(u08 chan)
{
  return offset;
}

void proto_api_write_offset(u08 chan, u32 off)
{
  offset = off;
}

u16 proto_api_read_mtu(u08 chan)
{
  return buf_words;
}

void proto_api_write_mtu(u08 chan, u16 mtu)
{
  if(mtu <= MAX_WORDS) {
    buf_words = mtu;
  }
}

int main(void)
{
  system_init();
  
  spi_init();
  
  uart_init();
  uart_send_pstring(PSTR("parbox: test-pamela!"));
  uart_send_crlf();

  // setup enc28j60
  uart_send_pstring(PSTR("enc28j60: init rev="));
  u08 rev;
  u08 ok = enc28j60_init(&rev);
  uart_send_hex_byte(rev);
  uart_send_pstring(PSTR(" ok="));
  uart_send_hex_byte(ok);
  uart_send_crlf();
  enc28j60_test_setup();

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
