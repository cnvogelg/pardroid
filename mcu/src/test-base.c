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

#include "proto_low.h"

int main(void)
{
  system_init();
  led_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-base!"));
  uart_send_crlf();

  rom_info();

  proto_low_init(0);

#ifdef TEST_WAIT_WATCHDOG
  // test wd
  for(int i=0;i<100;i++) {
    uart_send_hex_byte(i);
    uart_send_crlf();
    timer_delay(100);
  }
#endif

  while(1) {
    u08 on = 1;
    for(int i=0;i<10;i++) {
      system_wdt_reset();

      timer_delay(100);

      on = !on;
      led_set(on);
    }

#ifdef TEST_SYS_RESET
    uart_send_pstring(PSTR("reset!"));
    uart_send_crlf();
    system_sys_reset();
#endif
  }

  return 0;
}
