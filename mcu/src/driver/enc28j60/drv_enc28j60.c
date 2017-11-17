#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_DRIVER_ENC28J60

#include "debug.h"
#include "driver.h"
#include "drv_enc28j60.h"
#include "enc28j60.h"
#include  "uartutil.h"

static u08 init(u08 did)
{
  uart_send_pstring(PSTR("enc28j60: init: "));
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
    return DRIVER_INIT_FAILED;
  }
}

static u16 read(u08 did, u08 *buf, u16 size)
{
  DS("dNr:"); DW(size); DNL;
  return size;
}

static u16 write(u08 did, u08 *buf, u16 size)
{
  DS("dNw:"); DW(size); DNL;
  return size;
}

DRIVER_BEGIN(enc28j60)
  .init_func = init,
  .read_func = read,
  .write_func = write,
  .mtu_max = 0,
  .mtu_min = 0
DRIVER_END
