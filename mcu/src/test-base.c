#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "uart.h"
#include "uartutil.h"
#include "rominfo.h"
#include "system.h"

int main(void)
{
  system_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-base!"));
  uart_send_crlf();

  rom_info();

  while(1) {
    system_wdt_reset();
  }

  return 0;
}
