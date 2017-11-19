#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_DRIVER_SDCARD

#include "debug.h"
#include "driver.h"
#include "drv_sdcard.h"
#include "sdcard.h"
#include "uartutil.h"

static u08 init(u08 did)
{
  uart_send_pstring(PSTR("sdcard: "));
  u08 result = sdcard_init();
  if(result == SDCARD_RESULT_OK) {
    uart_send_pstring(PSTR("OK!"));
    uart_send_crlf();
    return DRIVER_OK;
  } else {
    uart_send_hex_byte(result);
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

DRIVER_BEGIN(sdcard)
  .init_func = init,
  .read_func = read,
  .write_func = write,
  .mtu_max = 0,
  .mtu_min = 0
DRIVER_END
