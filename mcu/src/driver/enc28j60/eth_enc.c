#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_DRIVER_ENC28J60

#include "debug.h"
#include "driver.h"
#include "eth_enc.h"
#include "enc28j60.h"
#include  "uartutil.h"

static u08 init(u08 did)
{
  uart_send_pstring(PSTR("eth_enc: "));
  u08 rev;
  u08 status = enc28j60_init(&rev);
  if(status == ENC28J60_RESULT_OK) {
    uart_send_pstring(PSTR("ok, rev="));
    uart_send_hex_byte(rev);
    uart_send_crlf();
    return DRIVER_OK;
  } else {
    uart_send_pstring(PSTR("NOT FOUND!"));
    uart_send_crlf();
    return DRIVER_ERROR_INIT_FAILED;
  }
}

DRIVER_BEGIN(eth_enc)
  .init_func = init,
  // consts
  .mtu_max = 0,
  .mtu_min = 0
DRIVER_END
