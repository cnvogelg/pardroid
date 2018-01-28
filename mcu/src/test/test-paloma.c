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

#include "handler_reg.h"

#include "pamela.h"
#include "paloma.h"

// define my app id
BASE_REG_APPID(FWID_TEST_PALOMA)

// set register table
REG_TABLE_SETUP(handler)

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
