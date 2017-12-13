#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "uart.h"
#include "uartutil.h"
#include "rominfo.h"
#include "system.h"
#include "led.h"
#include "timer.h"

#include "spi.h"

#include "driver.h"
#include "driver_list.h"

// driver
DRIVER_TABLE_BEGIN
#ifdef CONFIG_DRIVER_ENC28J60
  DRIVER_TABLE_ENTRY(eth_enc),
#endif
DRIVER_TABLE_END

int main(void)
{
  system_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-net!"));
  uart_send_crlf();

  rom_info();

  spi_init();

  DRIVER_RESET();
  DRIVER_INIT();

  // main loop
  while(1) {
    system_wdt_reset();
    DRIVER_WORK();
  }

  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  system_sys_reset();
  return 0;
}
