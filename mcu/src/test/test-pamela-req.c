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

#include "pamela_req.h"

#define TEST_MAX_ARG_SIZE  128
#define TEST_NUM_SLOTS     2
#define TEST_PORT          2000

FW_INFO(FWID_TEST_PAMELA_REQ, VERSION_TAG)

//#ifdef FLAVOR_DEBUG
//#define VERBOSE
//#endif

// buffers for each slot
struct slot {
  u08 buf[TEST_MAX_ARG_SIZE];
  u08 poll_count;
};


static struct slot slots[TEST_NUM_SLOTS];

// ----- req handler functions -----

static u08 my_begin(u08 chan, pamela_buf_t *buf)
{
  u08 slot_id = pamela_get_slot(chan);
  buf->data = slots[slot_id].buf;
  uart_send('<');
  return PAMELA_OK;
}

static u08 my_handle(u08 chan, pamela_buf_t *buf)
{
  u08 slot_id = pamela_get_slot(chan);
  slots[slot_id].poll_count = 0;

  uart_send('#');

  u08 *data = buf->data;
  u16 size = buf->size;
  for(u16 i=0;i<size;i++) {
    data[i] ++;
  }

  return PAMELA_POLL;
}

static u08 my_handle_poll(u08 chan, pamela_buf_t *buf)
{
  u08 slot_id = pamela_get_slot(chan);
  slots[slot_id].poll_count++;

  if(slots[slot_id].poll_count == 3) {
    uart_send('!');
    return PAMELA_OK;
  } else {
    uart_send('*');
    return PAMELA_POLL;
  }
}

static void my_end(u08 chan, pamela_buf_t *buf)
{
  // nop - no buffer free needed
  uart_send('>');
  uart_send_crlf();
}

// ----- define my req handler -----
REQ_HANDLER_BEGIN(my_handler, TEST_PORT, TEST_NUM_SLOTS, TEST_MAX_ARG_SIZE)
  .begin = my_begin,
  .handle = my_handle,
  .handle_poll = my_handle_poll,
  .end = my_end
REQ_HANDLER_END

REQ_HANDLER_TABLE_BEGIN
  &my_handler
REQ_HANDLER_TABLE_END

HANDLER_TABLE_BEGIN
  ADD_REQ_HANDLER(my_handler)
HANDLER_TABLE_END

int main(void)
{
  hw_system_init();
  hw_uart_init();

  uart_send_pstring(PSTR("parbox: test-pamela-req!"));
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
