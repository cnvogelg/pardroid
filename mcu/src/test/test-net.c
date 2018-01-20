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
#include "param.h"

#include "driver.h"

int main(void)
{
  system_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-net!"));
  uart_send_crlf();

  rom_info();

  param_init();

  spi_init();

  u08 num = DRIVER_GET_TABLE_SIZE();
  driver_reset(num);
  driver_init(num);

  // main loop
  while(1) {
    system_wdt_reset();
    driver_work(num);
  }

  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  system_sys_reset();
  return 0;
}
