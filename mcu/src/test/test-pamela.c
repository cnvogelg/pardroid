#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "debug.h"

#include "hw_uart.h"
#include "hw_system.h"
#include "hw_timer.h"

#include "uartutil.h"
#include "rominfo.h"
#include "fwid.h"
#include "fw_info.h"

#include "pamela.h"
#include "test/pamela.h"

FW_INFO(FWID_TEST_PAMELA, VERSION_TAG)

#ifdef FLAVOR_DEBUG
#define VERBOSE
#endif

static u08 read_buf[TEST_MAX_BUF_SIZE];
static u32 seek_pos = 0;

struct slot {
  u08 chan;
  u16 port;
  u16 delay;
  hw_timer_ms_t start;
  hw_timer_ms_t start_w;
};
struct slot slots[TEST_NUM_SLOTS];

// ----- handler functions -----

static u08 my_open(u08 chan, u16 port)
{
  u16 delay = port % 1000;
  u08 slot = pamela_get_slot(chan);

#ifdef VERBOSE
  uart_send_pstring(PSTR("<open:"));
  uart_send_hex_byte(slot);
  uart_send('@');
  uart_send_hex_byte(chan);
  uart_send(':');
  uart_send_hex_word(port);
  uart_send('+');
  uart_send_hex_word(delay);
  uart_send('>');
#endif

  if(port == TEST_OPEN_ERROR_PORT) {
    return PAMELA_WIRE_ERROR_OPEN;
  }

  slots[slot].chan = chan;
  slots[slot].port = port;
  slots[slot].delay = delay;
  slots[slot].start = hw_timer_millis();

  if(delay > 0) {
    return PAMELA_POLL;
  } else {
    return PAMELA_OK;
  }
}

static u08 my_open_poll(u08 chan, u16 port)
{
  u08 slot = pamela_get_slot(chan);

#ifdef VERBOSE
  uart_send('O');
#endif
  if(hw_timer_millis_timed_out(slots[slot].start, slots[slot].delay)) {
#ifdef VERBOSE
    uart_send('!');
#endif
    return PAMELA_OK;
  } else {
    return PAMELA_POLL;
  }
}

static u08 my_close(u08 chan)
{
  u08 slot = pamela_get_slot(chan);

#ifdef VERBOSE
  uart_send_pstring(PSTR("<close:"));
  uart_send_hex_byte(slot);
  uart_send('@');
  uart_send_hex_byte(chan);
  uart_send('>');
#endif

  slots[slot].start = hw_timer_millis();
  if(slots[slot].delay > 0) {
    return PAMELA_POLL;
  } else {
    return PAMELA_OK;
  }
}

static u08 my_close_poll(u08 chan)
{
  u08 slot = pamela_get_slot(chan);

#ifdef VERBOSE
  uart_send('C');
#endif
  if(hw_timer_millis_timed_out(slots[slot].start, slots[slot].delay)) {
#ifdef VERBOSE
    uart_send('!');
#endif
    return PAMELA_OK;
  } else {
    return PAMELA_POLL;
  }
}

static u08 my_reset(u08 chan)
{
  u08 slot = pamela_get_slot(chan);

#ifdef VERBOSE
  uart_send_pstring(PSTR("<reset:"));
  uart_send_hex_byte(slot);
  uart_send('@');
  uart_send_hex_byte(chan);
  uart_send('>');
#endif

  slots[slot].start = hw_timer_millis();
  if(slots[slot].delay > 0) {
    return PAMELA_POLL;
  } else {
    return PAMELA_OK;
  }
}

static u08 my_reset_poll(u08 chan)
{
  u08 slot = pamela_get_slot(chan);

#ifdef VERBOSE
  uart_send('R');
#endif
  if(hw_timer_millis_timed_out(slots[slot].start, slots[slot].delay)) {
#ifdef VERBOSE
    uart_send('!');
#endif
    return PAMELA_OK;
  } else {
    return PAMELA_POLL;
  }
}

// ----- seek/tell -----

void my_seek(u08 chan, u32 pos)
{
  u08 slot = pamela_get_slot(chan);

#ifdef VERBOSE
  uart_send_pstring(PSTR("<seek:"));
  uart_send_hex_byte(slot);
  uart_send('@');
  uart_send_hex_byte(chan);
  uart_send(':');
  uart_send_hex_long(pos);
  uart_send('>');
#endif
  seek_pos = pos;
}

u32 my_tell(u08 chan)
{
  u08 slot = pamela_get_slot(chan);

#ifdef VERBOSE
  uart_send_pstring(PSTR("<tell:"));
  uart_send_hex_byte(slot);
  uart_send('@');
  uart_send_hex_byte(chan);
  uart_send(':');
  uart_send_hex_long(seek_pos);
  uart_send('>');
#endif
  return seek_pos;
}

// ----- read -----

u08 my_read_request(u08 chan, pamela_buf_t *buf)
{
  u08 slot = pamela_get_slot(chan);

#ifdef VERBOSE
  uart_send_pstring(PSTR("<rr:"));
  uart_send_hex_byte(slot);
  uart_send('@');
  uart_send_hex_byte(chan);
  uart_send(':');
  uart_send_hex_word(buf->size);
  uart_send('>');
#endif

  // simulate read error on error channel
  if(buf->size == TEST_ERROR_REQ_SIZE) {
#ifdef VERBOSE
    uart_send_pstring(PSTR("ERR_REQ!"));
#endif
    return TEST_ERROR_READ;
  }

  // set read buffer
  buf->data = read_buf;

  slots[slot].start = hw_timer_millis();
  if((slots[slot].delay > 0) || (buf->size == TEST_ERROR_POLL_SIZE)) {
    return PAMELA_POLL;
  } else {
    return PAMELA_OK;
  }
}

u08 my_read_poll(u08 chan, pamela_buf_t *buf)
{
  u08 slot = pamela_get_slot(chan);

#ifdef VERBOSE
  uart_send('X');
#endif

  // simulate read error on error channel
  if(buf->size == TEST_ERROR_POLL_SIZE) {
#ifdef VERBOSE
    uart_send_pstring(PSTR("ERR_POLL!"));
#endif
    return TEST_ERROR_READ;
  }

  if(hw_timer_millis_timed_out(slots[slot].start, slots[slot].delay)) {
#ifdef VERBOSE
    uart_send('!');
#endif
    return PAMELA_OK;
  } else {
    return PAMELA_POLL;
  }
}

u08 my_read_done(u08 chan, pamela_buf_t *buf)
{
  u08 slot = pamela_get_slot(chan);

  // simulate read error on error channel
  if(buf->size == TEST_ERROR_DONE_SIZE) {
#ifdef VERBOSE
    uart_send_pstring(PSTR("ERR_DONE!"));
#endif
    return TEST_ERROR_READ;
  }

#ifdef VERBOSE
  uart_send_pstring(PSTR("<rd:"));
  uart_send_hex_byte(slot);
  uart_send('@');
  uart_send_hex_byte(chan);
  uart_send(':');
  uart_send_hex_word(buf->size);
  uart_send('>');
#endif

  return PAMELA_OK;
}

// ----- write -----

u08 my_write_request(u08 chan, pamela_buf_t *buf)
{
  u08 slot = pamela_get_slot(chan);

#ifdef VERBOSE
  uart_send_pstring(PSTR("<wr:"));
  uart_send_hex_byte(slot);
  uart_send('@');
  uart_send_hex_byte(chan);
  uart_send(':');
  uart_send_hex_word(buf->size);
  uart_send('>');
#endif

  // simulate write error on error channel
  if(buf->size == TEST_ERROR_REQ_SIZE) {
#ifdef VERBOSE
    uart_send_pstring(PSTR("ERR_REQ!"));
#endif
    return TEST_ERROR_WRITE;
  }

  // set write buffer
  buf->data = read_buf;

  slots[slot].start_w = hw_timer_millis();
  if((slots[slot].delay > 0) || (buf->size == TEST_ERROR_POLL_SIZE)) {
    return PAMELA_POLL;
  } else {
    return PAMELA_OK;
  }
}

u08 my_write_poll(u08 chan, pamela_buf_t *buf)
{
  u08 slot = pamela_get_slot(chan);

#ifdef VERBOSE
  uart_send('Y');
#endif

  // simulate read error on error channel
  if(buf->size == TEST_ERROR_POLL_SIZE) {
#ifdef VERBOSE
    uart_send_pstring(PSTR("ERR_POLL!"));
#endif
    return TEST_ERROR_WRITE;
  }

  if(hw_timer_millis_timed_out(slots[slot].start_w, slots[slot].delay)) {
#ifdef VERBOSE
    uart_send('!');
#endif
    return PAMELA_OK;
  } else {
    return PAMELA_POLL;
  }
}

u08 my_write_done(u08 chan, pamela_buf_t *buf)
{
  u08 slot = pamela_get_slot(chan);

  // simulate read error on error channel
  if(buf->size == TEST_ERROR_DONE_SIZE) {
#ifdef VERBOSE
    uart_send_pstring(PSTR("ERR_DONE!"));
#endif
    return TEST_ERROR_WRITE;
  }

#ifdef VERBOSE
  uart_send_pstring(PSTR("<wd:"));
  uart_send_hex_byte(slot);
  uart_send('@');
  uart_send_hex_byte(chan);
  uart_send(':');
  uart_send_hex_word(buf->size);
  uart_send('>');
#endif

  return PAMELA_OK;
}

u16 my_set_mtu(u08 chan, u16 new_mtu)
{
  u08 slot = pamela_get_slot(chan);

  u16 mtu;
  if(new_mtu < TEST_MAX_BUF_SIZE) {
    mtu = new_mtu;
  } else {
    mtu = TEST_MAX_BUF_SIZE;
  }

#ifdef VERBOSE
  uart_send_pstring(PSTR("<set_mtu:"));
  uart_send_hex_byte(slot);
  uart_send('@');
  uart_send_hex_byte(chan);
  uart_send(':');
  uart_send_hex_word(mtu);
  uart_send('>');
#endif
  return mtu;
}

// ----- define my handler -----
HANDLER_BEGIN(my_handler)
  // parameters
  .config.port_begin = TEST_PORT_MIN,
  .config.port_end = TEST_PORT_MAX,
  .config.def_mtu = TEST_DEFAULT_MTU,
  .config.max_mtu = TEST_MAX_BUF_SIZE,
  .config.max_slots = TEST_NUM_SLOTS,

  .open = my_open,
  .open_poll = my_open_poll,
  .close = my_close,
  .close_poll = my_close_poll,
  .reset = my_reset,
  .reset_poll = my_reset_poll,

  .set_mtu = my_set_mtu,

  .seek = my_seek,
  .tell = my_tell,

  .read_request = my_read_request,
  .read_poll = my_read_poll,
  .read_done = my_read_done,

  .write_request = my_write_request,
  .write_poll = my_write_poll,
  .write_done = my_write_done,
HANDLER_END

HANDLER_TABLE_BEGIN
  &my_handler
HANDLER_TABLE_END

int main(void)
{
  hw_system_init();
  hw_uart_init();

  uart_send_pstring(PSTR("parbox: test-pamela!"));
  uart_send_crlf();

  rom_info();

  pamela_init();

  while(1) {
    pamela_work();
#ifdef VERBOSE
    uart_send('.');
#endif
 }

  return 0;
}
