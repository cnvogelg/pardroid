#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "hw_uart.h"
#include "hw_system.h"

#include "uartutil.h"
#include "rominfo.h"
#include "fwid.h"
#include "fw_info.h"

#include "pamela.h"

FW_INFO(FWID_TEST_PAMELA, VERSION_TAG)

#define MAX_SIZE        512
#define ERROR_CHANNEL   15

static u08 read_buf[MAX_SIZE];
static u32 seek_pos = 0;

// ----- handler functions -----

static void my_work(u16 channel_mask)
{
#if 0
  uart_send_pstring(PSTR("[work]"));
  uart_send_hex_word(channel_mask);
  uart_send_crlf();
#endif
}

static void my_open(u08 chn, u16 port)
{
  uart_send_pstring(PSTR("[open]"));
  uart_send_hex_byte(chn);
  uart_send(':');
  uart_send_hex_word(port);
  uart_send_crlf();
  pamela_open_done(chn, PAMELA_OK);
}

static void my_close(u08 chn)
{
  uart_send_pstring(PSTR("[close]"));
  uart_send_hex_byte(chn);
  uart_send_crlf();
  pamela_close_done(chn, PAMELA_OK);
}

static void my_reset(u08 chn)
{
  uart_send_pstring(PSTR("[reset]"));
  uart_send_hex_byte(chn);
  uart_send_crlf();
  pamela_reset_done(chn);
}

// ----- seek/tell -----

void my_seek(u08 chn, u32 pos)
{
  uart_send_pstring(PSTR("[seek]"));
  uart_send_hex_byte(chn);
  uart_send(':');
  uart_send_hex_long(pos);
  uart_send_crlf();
  seek_pos = pos;
}

u32 my_tell(u08 chn)
{
  uart_send_pstring(PSTR("[tell]"));
  uart_send_hex_byte(chn);
  uart_send(':');
  uart_send_hex_long(seek_pos);
  uart_send_crlf();
  return seek_pos;
}

// ----- read -----

u08 my_read_request(u08 chn, u16 size)
{
  uart_send_pstring(PSTR("[r:"));
  uart_send_hex_word(size);
  uart_send(']');
  uart_send_hex_byte(chn);
  uart_send_crlf();

  // simulate read error on error channel
  if(chn == ERROR_CHANNEL) {
    pamela_read_error(chn);
    // say OK here
    return PAMELA_OK;
  }

  // reply immediately
  pamela_read_reply(chn, read_buf, size);

  return PAMELA_OK;
}

void my_read_done(u08 chn, u08 *buf, u16 size)
{
  uart_send_pstring(PSTR("[R:"));
  uart_send_hex_word(size);
  uart_send(']');
  uart_send_hex_byte(chn);
  uart_send_crlf();
}

// ----- write -----

u08 my_write_request(u08 chn, u16 size)
{
  uart_send_pstring(PSTR("[w:"));
  uart_send_hex_word(size);
  uart_send(']');
  uart_send_hex_byte(chn);
  uart_send_crlf();

  // simulate read error on error channel
  if(chn == ERROR_CHANNEL) {
    pamela_write_error(chn);
    // say OK here
    return PAMELA_OK;
  }

  // reply immediately
  pamela_write_reply(chn, read_buf, size);

  return PAMELA_OK;
}

void my_write_done(u08 chn, u08 *buf, u16 size)
{
  uart_send_pstring(PSTR("[W:]"));
  uart_send_hex_word(size);
  uart_send(']');
  uart_send_hex_byte(chn);
  uart_send_crlf();
}

u16 my_set_mtu(u08 chn, u16 new_mtu)
{
  u16 mtu;
  if(new_mtu < MAX_SIZE) {
    mtu = new_mtu;
  } else {
    mtu = MAX_SIZE;
  }

  uart_send_pstring(PSTR("[set_mtu]"));
  uart_send_hex_byte(chn);
  uart_send(':');
  uart_send_hex_word(mtu);
  uart_send_crlf();
  return mtu;
}

// ----- define my handler -----
HANDLER_BEGIN(my_handler)
  // parameters
  .config.port_begin = 1234,
  .config.port_end = 2345,
  .config.def_mtu = 128,
  .config.max_mtu = MAX_SIZE,

  // functions
  .work = my_work,

  .open = my_open,
  .close = my_close,
  .reset = my_reset,

  .set_mtu = my_set_mtu,

  .seek = my_seek,
  .tell = my_tell,

  .read_request = my_read_request,
  .read_done = my_read_done,

  .write_request = my_write_request,
  .write_done = my_write_done,
HANDLER_END


int main(void)
{
  hw_system_init();
  hw_uart_init();

  uart_send_pstring(PSTR("parbox: test-pamela!"));
  uart_send_crlf();

  rom_info();

  pamela_init();
  pamela_add_handler(&my_handler);

  while(1) {
    hw_system_wdt_reset();
    pamela_work();
    uart_send('.');
 }

  return 0;
}
