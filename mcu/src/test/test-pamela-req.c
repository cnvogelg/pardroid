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
};


static struct slot slots[TEST_NUM_SLOTS];

// ----- req handler functions -----

static u08 my_begin(u08 chan, u08 **req_buf, u16 req_size)
{
  u08 slot = pamela_get_slot(chan);
  *req_buf = slots[slot].buf;
  return PAMELA_OK;
}

static u08 my_handle(u08 chan, u08 *req_buf, u16 req_size, u08 **rep_buf, u16 *rep_size)
{
  // increment each byte
  for(int i=0;i<req_size;i++) {
    req_buf[i] += 1;
  }

  // simply return request buf as reply
  *rep_buf = req_buf;
  *rep_size = req_size;
  return PAMELA_OK;
}

static void my_end(u08 chan, u08 *rep_buf, u16 rep_size)
{
  // nop - no buffer free needed
}

// ----- define my req handler -----
REQ_HANDLER_BEGIN(my_handler, TEST_PORT, TEST_NUM_SLOTS, TEST_MAX_ARG_SIZE)
  .begin = my_begin,
  .handle = my_handle,
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
