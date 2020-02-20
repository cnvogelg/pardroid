#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "uart.h"
#include "uartutil.h"
#include "rominfo.h"
#include "fwid.h"
#include "fw_info.h"
#include "system.h"

#include "pamela.h"

FW_INFO(FWID_TEST_PAMELA, VERSION_TAG)

// my channels
channel_t my_channels[2];

// access channels
channel_ptr_t channel_api_get_channel(u08 id)
{
  if(id < 2) {
    return &my_channels[id];
  } else {
    return NULL;
  }
}

u16 channel_api_get_mask(void)
{
  return 0x03; // bit 0,1 are the active channels
}

// buffer
#define MAX_WORDS 512
static u08 buf[MAX_WORDS * 2];

// handler functions
u08 my_open(u08 chn)
{
  uart_send_pstring(PSTR("[open]"));
  uart_send_hex_byte(chn);
  uart_send_crlf();
  return 0; // OK
}

void my_work(u08 chn)
{
  uart_send('.');
}

void my_close(u08 chn)
{
  uart_send_pstring(PSTR("[close]"));
  uart_send_hex_byte(chn);
  uart_send_crlf();
}

void my_reset(u08 chn)
{
  uart_send_pstring(PSTR("[reset]"));
  uart_send_hex_byte(chn);
  uart_send_crlf();
}

u16 my_set_mtu(u08 chn, u16 new_mtu)
{
  uart_send_pstring(PSTR("[mtu]"));
  uart_send_hex_byte(chn);
  uart_send(':');
  uart_send_hex_word(new_mtu);
  uart_send_crlf();
  return new_mtu;
}

// ----- read -----

u16 my_read_begin(u08 chn, u16 mtu, u32 offset)
{
  uart_send_pstring(PSTR("[r:mtu="));
  uart_send_hex_word(mtu);
  uart_send_pstring(PSTR(",of="));
  uart_send_hex_long(offset);
  uart_send(']');
  uart_send_hex_byte(chn);
  uart_send_crlf();
  return MAX_WORDS;
}

u08 *my_read_chunk_begin(u08 chn, u16 off, u16 size)
{
  uart_send_pstring(PSTR("[R@"));
  uart_send_hex_word(off);
  uart_send('+');
  uart_send_hex_word(size);
  uart_send(']');
  uart_send_hex_byte(chn);
  uart_send_crlf();
  return &buf[off<<1]; 
}

void my_read_chunk_end(u08 chn)
{
  uart_send_pstring(PSTR("[R!]"));
  uart_send_hex_byte(chn);
  uart_send_crlf();
}

void my_read_end(u08 chn, u08 cancelled)
{
  uart_send_pstring(PSTR("[r!]"));
  uart_send_hex_byte(chn);
  uart_send(':');
  uart_send_hex_byte(cancelled);
  uart_send_crlf();
}

// ----- write -----

void my_write_begin(u08 chn, u16 mtu, u32 offset, u16 size)
{
  uart_send_pstring(PSTR("[w:mtu="));
  uart_send_hex_word(mtu);
  uart_send_pstring(PSTR(",of="));
  uart_send_hex_long(offset);
  uart_send('+');
  uart_send_hex_word(size);
  uart_send(']');
  uart_send_hex_byte(chn);
  uart_send_crlf();
}

u08 *my_write_chunk_begin(u08 chn, u16 off, u16 size)
{
  uart_send_pstring(PSTR("[W@"));
  uart_send_hex_word(off);
  uart_send('+');
  uart_send_hex_word(size);
  uart_send(']');
  uart_send_hex_byte(chn);
  uart_send_crlf();
  return &buf[off<<1]; 
}

void my_write_chunk_end(u08 chn)
{
  uart_send_pstring(PSTR("[W!]"));
  uart_send_hex_byte(chn);
  uart_send_crlf();
}

void my_write_end(u08 chn, u08 cancelled)
{
  uart_send_pstring(PSTR("[w!]"));
  uart_send_hex_byte(chn);
  uart_send(':');
  uart_send_hex_byte(cancelled);
  uart_send_crlf();
}

// ----- define my handler -----
HANDLER_BEGIN(my_handler)
  // parameters
  .def_mtu = 64,
  .max_words = MAX_WORDS,
  .mode = CHANNEL_MODE_NONE,
  // functions
  .open = my_open,
  .work = my_work,
  .close = my_close,
  .reset = my_reset,

  .read_begin = my_read_begin,
  .read_chunk_begin = my_read_chunk_begin,
  .read_chunk_end = my_read_chunk_end,
  .read_end = my_read_end,

  .write_begin = my_write_begin,
  .write_chunk_begin = my_write_chunk_begin,
  .write_chunk_end = my_write_chunk_end,
  .write_end = my_write_end,

  .set_mtu = my_set_mtu
HANDLER_END


int main(void)
{
  system_init();
  
  uart_init();
  uart_send_pstring(PSTR("parbox: test-pamela!"));
  uart_send_crlf();

  rom_info();

  pamela_init();
  channel_init(&my_channels[0], &my_handler);
  channel_init(&my_channels[1], &my_handler);

  while(1) {
    system_wdt_reset();
    pamela_handle();
 }

  return 0;
}
