#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "system.h"
#include "rominfo.h"
#include "fwid.h"
#include "uart.h"
#include "uartutil.h"
#include "base_reg.h"
#include "mem.h"

#include "handler.h"
#include "handler_reg.h"
#include "hnd_echo.h"
#include "hnd_null.h"

#include "driver.h"
#include "driver_list.h"

#include "pamela.h"
#include "paloma.h"

// define my app id
BASE_REG_APPID(FWID_TEST_HANDLER)

// set register table
REG_TABLE_SETUP(handler)

// handler
HANDLER_TABLE_BEGIN
  HANDLER_TABLE_ENTRY(echo),
  HANDLER_TABLE_ENTRY(echo),
  HANDLER_TABLE_ENTRY(null)
HANDLER_TABLE_END

// driver
DRIVER_TABLE_BEGIN
  DRIVER_TABLE_ENTRY(blk_null),
#ifdef CONFIG_DRIVER_ENC28J60
  DRIVER_TABLE_ENTRY(eth_enc),
#endif
#ifdef CONFIG_DRIVER_SDCARD
  DRIVER_TABLE_ENTRY(blk_sdraw),
#endif
DRIVER_TABLE_END


int main(void)
{
  system_init();
  mem_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-paloma!"));
  uart_send_crlf();

  rom_info();

  pamela_init();
  paloma_init();

  while(1) {
    system_wdt_reset();
    pamela_handle();
    paloma_handle();
  }

  return 0;
}
